#ifndef SWCL_H
#define SWCL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <EGL/egl.h>
#include <wayland-client.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------- MACROS -------- //

// Log formatted message
#define SWCL_LOG(format, ...)                                                  \
  fprintf(stderr, "SWCL: " format "\n", ##__VA_ARGS__)

// Debug log formatted message
#ifdef SWCL_ENABLE_DEBUG_LOGS
#define SWCL_LOG_DEBUG(format, ...)                                            \
  fprintf(stdout, "SWCL DEBUG: " format "\n", ##__VA_ARGS__)
#else
#define SWCL_LOG_DEBUG(format, ...) ((void)0)
#endif

// Not implemented message
#define SWCL_NOT_IMPLEMENTED(func_name)                                        \
  printf("SWCL FUNCTION IS NOT IMPLEMENTED: %s\n", func_name)

// Exit program with message
#define SWCL_PANIC(format, ...)                                                \
  do {                                                                         \
    fprintf(stderr, "SWCL PANIC: " format "\n", ##__VA_ARGS__);                \
    exit(1);                                                                   \
  } while (0)

// ---------- UTILS ---------- //

// Function to generate unique ID
static inline int swcl_generate_id() {
  static int id = -1; // static variable to hold the current ID value
  return ++id;        // increment and return the current ID
}

// --- Dynamic Array --- //

// Simple dynamic array structure that can only be grown in size
typedef struct {
  uint32_t length;
  uint32_t capacity;
  void **items;
} SWCLArray;

// Create new dynamic array with given initial capacity.
static inline SWCLArray swcl_array_new(uint32_t initial_capacity) {
  SWCLArray a = {
      .length = 0,
      .capacity = initial_capacity,
      .items = (void **)malloc(sizeof(void *) * initial_capacity),
  };
  return a;
};

// Add item to the end of the array, resizing it if needed.
static inline void swcl_array_append(SWCLArray *array, void *item) {
  if (array->length + 1 > array->capacity) {
    void **new_array =
        (void **)realloc(array->items, ++array->capacity * sizeof(void *));
    array->items = new_array;
  }
  array->items[array->length++] = item;
}

// Destroy array
static inline void swcl_array_free(SWCLArray array) {
  for (uint32_t i = 0; i < array.length; i++) {
    free(array.items[i]);
  }
}

// ---------- ENUMS ---------- //

// Direction of the mouse scroll wheel
typedef enum {
  SWCL_SCROLL_UP = 0,
  SWCL_SCROLL_DOWN = 1,
} SWCLScrollDirection;

// Mouse buttons keycodes
typedef enum {
  SWCL_MOUSE_1 = 272, // Left
  SWCL_MOUSE_2 = 273, // Right
  SWCL_MOUSE_3 = 274, // Middle
  SWCL_MOUSE_4 = 275, // Back
  SWCL_MOUSE_5 = 276, // Forward
} SWCLMouseButton;

// State of the button
typedef enum {
  SWCL_BUTTON_RELEASED = 0,
  SWCL_BUTTON_PRESSED = 1,
} SWCLButtonState;

// Edge or corner of the window. Passed to 'swcl_window_resize' function.
typedef enum {
  SWCL_WINDOW_EDGE_NONE = 0,
  SWCL_WINDOW_EDGE_TOP = 1,
  SWCL_WINDOW_EDGE_BOTTOM = 2,
  SWCL_WINDOW_EDGE_LEFT = 4,
  SWCL_WINDOW_EDGE_TOP_LEFT = 5,
  SWCL_WINDOW_EDGE_BOTTOM_LEFT = 6,
  SWCL_WINDOW_EDGE_RIGHT = 8,
  SWCL_WINDOW_EDGE_TOP_RIGHT = 9,
  SWCL_WINDOW_EDGE_BOTTOM_RIGHT = 10,
} SWCLWindowEdge;

// Ancor position of the window.
typedef enum {
  SWCL_ANCOR_NONE = 0,
  SWCL_ANCOR_TOP = 1,
  SWCL_ANCOR_BOTTOM = 2,
  SWCL_ANCOR_LEFT = 3,
  SWCL_ANCOR_RIGHT = 4,
  SWCL_ANCOR_CENTER = 5,
} SWCLAncor;

// ---------- STRUCTS ---------- //

typedef struct SWCLWindow SWCLWindow;
typedef struct SWCLApplication SWCLApplication;

// Toplevel window object
struct SWCLWindow {
  // Read-Only properties
  uint32_t id;
  char *title;
  uint32_t width;
  uint32_t height;
  uint32_t min_width;
  uint32_t min_height;
  bool maximized;
  bool fullscreen;

  // Draw function
  void (*on_draw_cb)(SWCLWindow *win);

  // Wayland elements
  struct wl_surface *wl_surface;
  struct wl_callback *wl_callback;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;

  // EGL
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;

  SWCLApplication *app;
};

// SWCL application configuration.
// Create before initializing.
// app_id must be in format e. g. "com.mydomain.AppName"
// Callbacks can be NULL.
typedef struct {
  const char *app_id;
  void (*on_pointer_enter_cb)(SWCLWindow *win, int x, int y);
  void (*on_pointer_leave_cb)(SWCLWindow *win);
  void (*on_pointer_motion_cb)(SWCLWindow *win, int x, int y);
  void (*on_mouse_button_cb)(SWCLWindow *win, SWCLMouseButton button,
                             SWCLButtonState state);
  void (*on_mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*on_keyboard_key_cb)(SWCLWindow *win, uint32_t key,
                             SWCLButtonState state);
  void (*on_keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked,
                                 uint32_t group);
} SWCLConfig;

// Position with x and y coordinates
typedef struct {
  uint32_t x;
  uint32_t y;
} SWCLPoint;

struct SWCLApplication {
  // Properties
  const char *app_id;
  bool running;
  SWCLArray windows;
  SWCLPoint cursor_pos;
  SWCLWindow *current_window;

  // Serials of events
  uint32_t wl_pointer_serial;
  uint32_t wl_keyboard_serial;

  // Wayland
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct wl_seat *wl_seat;
  struct wl_pointer *wl_pointer;
  struct wl_keyboard *wl_keyboard;
  struct xdg_wm_base *xdg_wm_base;
  struct zwlr_layer_shell_v1 *wlr_layer_shell;

  EGLConfig egl_config;
  EGLDisplay egl_display;
  EGLContext egl_context;

  // Cursor
  struct wl_buffer *wl_cursor_buffer;
  struct wl_cursor *wl_cursor;
  struct wl_cursor_theme *wl_cursor_theme;
  struct wl_cursor_image *wl_cursor_image;
  struct wl_shm *wl_cursor_shm;
  struct wl_surface *wl_cursor_surface;
  char *current_cursor_name;

  // Callbacks
  void (*on_pointer_enter_cb)(SWCLWindow *win, int x, int y);
  void (*on_pointer_leave_cb)(SWCLWindow *win);
  void (*on_pointer_motion_cb)(SWCLWindow *win, int x, int y);
  void (*on_mouse_button_cb)(SWCLWindow *win, SWCLMouseButton button,
                             SWCLButtonState state);
  void (*on_mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*on_keyboard_key_cb)(SWCLWindow *win, uint32_t key,
                             SWCLButtonState state);
  void (*on_keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked,
                                 uint32_t group);
};

// ---------- DRAWING-RELATED STRUCTS ---------- //

// RGBA color. Values can be from 0 to 255.
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} SWCLColor;

// Rectangle with top left corner position at 'x' and 'y',
// width 'w' and height 'h'
typedef struct {
  int x;
  int y;
  int w;
  int h;
} SWCLRect;

// Circle where 'cx' and 'cy' are coordinates of the center of the circle and
// 'r' is the radius
typedef struct {
  int cx;
  int cy;
  uint32_t r;
} SWCLCircle;

// ---------- APPLICATION ---------- //

// Initialize SWCL.
// It will initialize connection to Wayland display, register mouse, keyboard
// and touch devices. Also it will initialize EGL and create OpenGL context.
SWCLApplication *swcl_application_new(SWCLConfig *cfg);

// Start the application loop
void swcl_application_run(SWCLApplication *app);

// Set cursor image with given name and size.
// Name can be one of those:
// "left_ptr" - default cursor
// "top_side" - resize top side
// "bottom_side" - resize bottom side
// "left_side" - resize left side
// "right_side" - resize right side
// "top_left_corner" - resize top left corner
// "top_right_corner" - resize top right corner
// "bottom_left_corner" - resize bottom left corner
// "bottom_right_corner" - resize bottom right corner
void swcl_application_set_cursor(SWCLApplication *app, const char *name,
                                 uint8_t size);

// Shutdown SWCL application
void swcl_application_quit(SWCLApplication *app);

// ---------- WINDOW ---------- //

// Create new window.
// This function takes care of creating native wayland window with stuff like
// wl_surface, xdg_surface, xdg_toplevel and putting egl_window with OpenGL
// context into it.
SWCLWindow *swcl_window_new(SWCLApplication *app, char *title, uint16_t width,
                            uint16_t height, uint16_t min_width,
                            uint16_t min_height, bool maximized,
                            bool fullscreen,
                            void (*draw_func)(SWCLWindow *win));

// Set window ancor. Useful for bars, docks or run menus types of apps.
// Can be used only with compositors that support
// wlr_layer_shell protocol, e. g. hyprland, kwin, sway.
// If compositor is not supported - does nothing.
void swcl_window_ancor(SWCLAncor ancor);

// Request server-side decorations (SSR) for the window.
// Compositor must support xdg_decoration protocol.
// Supported compositors: kwin, sway, hyprland.
void swcl_window_request_ssr(SWCLWindow *win);

// Tells compositor to begin native drag operation. With this, window can be
// snapped to the sides of the screen if comositor allows it. This function is
// useful for implementing Client-Side Decorations (CSD).
void swcl_window_drag(SWCLWindow *win);

// Tells compositor to begin native resize operation. With this, window can be
// resized if comositor allows it.
// This function is useful for implementing Client-Side Decorations (CSD).
void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge);

// Swap OpenGL buffer for rendered frame
void swcl_window_swap_buffers(SWCLWindow *win);

// Set window properties

// Set window title
void swcl_window_set_title(SWCLWindow *win, char *title);

// Set window size
// void swcl_window_set_size(SWCLWindow *win, int width, int height);

// Set window minimum size
void swcl_window_set_min_size(SWCLWindow *win, int min_width, int min_height);

// Set window maximized state
void swcl_window_set_maximized(SWCLWindow *win, bool maximized);

// Set window minimized state
void swcl_window_minimize(SWCLWindow *win);

// Set window fullscreen state
void swcl_window_set_fullscreen(SWCLWindow *win, bool maximized);

// ---------- DRAWING ---------- //

// Clear buffer background
void swcl_clear_background(SWCLColor color);

// Draw rectangle with given color and dimentions.
void swcl_draw_rect(SWCLColor color, SWCLRect rect);

// Same as swcl_draw_rect, but with rounded corners
void swcl_draw_rounded_rect(SWCLColor color, SWCLRect rect, int radius);

// Draw circle
void swcl_draw_circle(SWCLColor color, SWCLCircle circle);

#ifdef __cplusplus
}
#endif

#endif // SWCL_H
