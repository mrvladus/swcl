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

// Logging
#define SWCL_LOG(format, ...)                                                  \
  fprintf(stderr, "SWCL: " format "\n", ##__VA_ARGS__)

#ifdef SWCL_ENABLE_DEBUG_LOGS
#define SWCL_LOG_DEBUG(format, ...)                                            \
  fprintf(stdout, "SWCL DEBUG: " format "\n", ##__VA_ARGS__)
#else
#define SWCL_LOG_DEBUG(format, ...) ((void)0)
#endif

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
  int length;
  int capacity;
  void **items;
} SWCLArray;

// Create new dynamic array with given initial capacity.
static inline SWCLArray swcl_array_new(int initial_capacity) {
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
static inline void swcl_array_free(SWCLArray *array) {
  for (uint32_t i = 0; i < array->length; i++) {
    free(&array[i]);
  }
  free(array);
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

typedef enum {
  SWCL_WINDOW_FLAG_MAXIMIZED = 0x1,
  SWCL_WINDOW_FLAG_FULLSCREEN = 0x2,
} SWCLWindowFlags;

// ---------- STRUCTS ---------- //

typedef struct SWCLWindow SWCLWindow;

typedef struct SWCLCursor {
  struct wl_shm *wl_shm;
  struct wl_buffer *wl_buffer;
  struct wl_surface *wl_surface;
  struct wl_cursor_theme *wl_cursor_theme;
  struct wl_cursor_image *wl_cursor_image;
  struct wl_cursor *wl_cursor;
} SWCLCursor;

// SWCL application configuration.
// Create before initializing.
// app_id must be in format e. g. "com.mydomain.AppName"
// Callbacks can be NULL.
typedef struct {
  const char *app_id;
  void (*on_pointer_enter_cb)(SWCLWindow *win, int x, int y, uint32_t serial);
  void (*on_pointer_leave_cb)(SWCLWindow *win, uint32_t serial);
  void (*on_pointer_motion_cb)(SWCLWindow *win, int x, int y);
  void (*on_mouse_button_cb)(SWCLWindow *win, SWCLMouseButton button,
                             SWCLButtonState state, uint32_t serial);
  void (*on_mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*on_keyboard_key_cb)(SWCLWindow *win, uint32_t key,
                             SWCLButtonState state, uint32_t serial);
  void (*on_keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked,
                                 uint32_t group, uint32_t serial);
} SWCLConfig;

// Position with x and y coordinates
typedef struct {
  int x;
  int y;
} SWCLPoint;

// ---------- GLOBALS ---------- //

// Wayland specific variables (Use directly only if you know what you doing)
extern struct wl_display *swcl_wl_display;
extern struct wl_registry *swcl_wl_registry;
extern struct wl_compositor *swcl_wl_compositor;
extern struct wl_seat *swcl_wl_seat;
extern struct wl_pointer *swcl_wl_pointer;
extern struct wl_keyboard *swcl_wl_keyboard;
extern struct xdg_wm_base *swcl_xdg_wm_base;

// EGL specific variables (Use directly only if you know what you doing)
extern EGLConfig swcl_egl_config;
extern EGLDisplay swcl_egl_display;
extern EGLContext swcl_egl_context;

// Application ID
extern const char *swcl_app_id;
// Application run state
extern bool swcl_app_running;
// Cursor position
extern SWCLPoint swcl_cursor_pos;
// Current window ID
extern int swcl_current_window_id;
// Dinamic array of windows
extern SWCLArray swcl_windows;

// ---------- INITIALIZATION ---------- //

// Initialize SWCL.
// It will initialize connection to Wayland display, register mouse, keyboard
// and touch devices. Also it will initialize EGL and create OpenGL context.
bool swcl_init(SWCLConfig *cfg);

// Start the application loop
void swcl_run();

// Shutdown SWCL application
void swcl_quit();

// ---------- CURSOR ---------- //

void swcl_set_cursor(const char *name, uint8_t size, uint32_t serial);

// ---------- WINDOW ---------- //

// Toplevel window object
struct SWCLWindow {
  // Props
  int id;
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

  SWCLCursor cursor;

  // EGL
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;
};

typedef struct {
  // Properties
  char *title;
  uint32_t width;
  uint32_t height;
  uint32_t min_width;
  uint32_t min_height;
  bool maximized;
  bool fullscreen;

  // Draw callback
  void (*on_draw_cb)(SWCLWindow *win);
} SWCLWindowConfig;

// Create new window.
// This function takes care of creating native wayland window with stuff like
// wl_surface, xdg_surface, xdg_toplevel and putting egl_window with OpenGL
// context into it.
SWCLWindow *swcl_window_new(SWCLWindowConfig cfg);

// Tells compositor to begin native drag operation. With this window can be
// snapped to the sides if comositor allows it. This function is useful for
// implementing Client-Side Decorations (CSD).
//
// serial - is event id. It can be taken from "mouse-button" callback.
void swcl_window_drag(SWCLWindow *win, uint32_t serial);

// Tells compositor to begin native resize operation. With this window can be
// resized if comositor allows it.
// This function is useful for implementing Client-Side Decorations (CSD).
//
// serial - is event id. It can be taken from "mouse-button" callback.
void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge, uint32_t serial);

// Swap OpenGL buffer for rendered frame
void swcl_window_swap_buffers(SWCLWindow *win);

// Get window properties

// ID property
int swcl_window_get_id(SWCLWindow *win);

// Title property
char *swcl_window_get_title(SWCLWindow *win);

// Width property
int swcl_window_get_width(SWCLWindow *win);

// Height property
int swcl_window_get_height(SWCLWindow *win);

// Minimum width property
int swcl_window_get_min_width(SWCLWindow *win);

// Minimum height property
int swcl_window_get_min_height(SWCLWindow *win);

// Maximized state property
bool swcl_window_get_maximized(SWCLWindow *win);

// Fulscreen state property
bool swcl_window_get_fullscreen(SWCLWindow *win);

// Set window properties

// Set window title
void swcl_window_set_title(SWCLWindow *win, char *title);

// Set window size
void swcl_window_set_min_size(SWCLWindow *win, int width, int height);

// Set window minimum size
void swcl_window_set_min_size(SWCLWindow *win, int min_width, int min_height);

// Set window maximized state
void swcl_window_set_maximized(SWCLWindow *win, bool maximized);

// Set window fullscreen state
void swcl_window_set_fullscreen(SWCLWindow *win, bool maximized);

// Set cursor image from name
void swcl_window_set_cursor(SWCLWindow *win, const char *name, uint32_t serial);

// ---------- DRAWING ---------- //

// Clear window with RGBA color
void swcl_clear_background(float r, float g, float b, float a);

#ifdef __cplusplus
}
#endif

#endif // SWCL_H
