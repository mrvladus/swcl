// Copyright 2024 Vlad Krupinskii <mrvladus@yandex.ru>
// SPDX-License-Identifier: MIT

// SWCL - Simple Wayland Client Library
// Create Wayland clients easily.
// This header-only library simplifies creating native Wayland window, receiving
// mouse or keyboard events and helping with creation of Client-Side Decorations
// (CSD).

// Compile flags: -lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm

#ifndef SWCL_H
#define SWCL_H

#include <EGL/egl.h>
#include <GL/gl.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define SWCL_ALLOC(T) (T *)malloc(sizeof(T))

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

typedef struct SWCLApplication SWCLApplication;
typedef struct SWCLWindow SWCLWindow;

// Toplevel window object
typedef struct SWCLWindow {
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
} SWCLWindow;

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
  uint32_t x;
  uint32_t y;
  uint32_t w;
  uint32_t h;
} SWCLRect;

// Circle where 'cx' and 'cy' are coordinates of the center of the circle and
// 'r' is the radius
typedef struct {
  uint32_t cx;
  uint32_t cy;
  uint32_t r;
} SWCLCircle;

// ---------- APPLICATION ---------- //

// Initialize SWCL.
// It will initialize connection to Wayland display, register mouse, keyboard
// and touch devices. Also it will initialize EGL and create OpenGL context.
static SWCLApplication *swcl_application_new(SWCLConfig *cfg);

// Start the application loop
static void swcl_application_run(SWCLApplication *app);

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
static void swcl_application_set_cursor(SWCLApplication *app, const char *name,
                                        uint8_t size);

// Shutdown SWCL application
static void swcl_application_quit(SWCLApplication *app);

// ---------- WINDOW ---------- //

// Create new window.
// This function takes care of creating native wayland window with stuff like
// wl_surface, xdg_surface, xdg_toplevel and putting egl_window with OpenGL
// context into it.
static SWCLWindow *swcl_window_new(SWCLApplication *app, char *title,
                                   uint16_t width, uint16_t height,
                                   uint16_t min_width, uint16_t min_height,
                                   bool maximized, bool fullscreen,
                                   void (*draw_func)(SWCLWindow *win));

// Start window rendering.
static void swcl_window_show(SWCLWindow *win);

// Set window ancor. Useful for bars, docks or run menus types of apps.
// Can be used only with compositors that support
// wlr_layer_shell protocol, e. g. hyprland, kwin, sway.
// If compositor is not supported - does nothing.
static void swcl_window_ancor(SWCLAncor ancor);

// Request server-side decorations (SSR) for the window.
// Compositor must support xdg_decoration protocol.
// Supported compositors: kwin, sway, hyprland.
static void swcl_window_request_ssr(SWCLWindow *win);

// Show compositor window menu. This function is
// useful for implementing Client-Side Decorations (CSD).
static void swcl_window_show_menu(SWCLWindow *win);

// Tells compositor to begin native drag operation. With this, window can be
// snapped to the sides of the screen if comositor allows it. This function is
// useful for implementing Client-Side Decorations (CSD).
static void swcl_window_drag(SWCLWindow *win);

// Tells compositor to begin native resize operation. With this, window can be
// resized if comositor allows it.
// This function is useful for implementing Client-Side Decorations (CSD).
static void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge);

// Swap OpenGL buffer for rendered frame
static void swcl_window_swap_buffers(SWCLWindow *win);

// Set window properties

// Set window title
static void swcl_window_set_title(SWCLWindow *win, char *title);

// Set window size
// void swcl_window_set_size(SWCLWindow *win, int width, int height);

// Set window minimum size
static void swcl_window_set_min_size(SWCLWindow *win, int min_width,
                                     int min_height);

// Set window maximized state
static void swcl_window_set_maximized(SWCLWindow *win, bool maximized);

// Set window minimized state
static void swcl_window_minimize(SWCLWindow *win);

// Set window fullscreen state
static void swcl_window_set_fullscreen(SWCLWindow *win, bool maximized);

// ---------- DRAWING ---------- //

// Clear buffer background
static void swcl_clear_background(SWCLColor color);

// Draw rectangle with given color and dimentions.
static void swcl_draw_rect(SWCLColor color, SWCLRect rect);

// Same as swcl_draw_rect, but with rounded corners
static void swcl_draw_rounded_rect(SWCLColor color, SWCLRect rect, int radius);

// Draw circle
static void swcl_draw_circle(SWCLColor color, SWCLCircle circle);

// ------------------------------------------------------------------------- //
//                                                                           //
//                       XDG_SHELL_PROTOCOL DEFENITION                       //
//                        INSERTED DURING BUILD STEP                         //
//                                                                           //
// ------------------------------------------------------------------------- //

#include <stddef.h>
#include <stdint.h>
#include <wayland-client.h>
struct wl_output;
struct wl_seat;
struct wl_surface;
struct xdg_popup;
struct xdg_positioner;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_wm_base;
#ifndef XDG_WM_BASE_INTERFACE
#define XDG_WM_BASE_INTERFACE
extern const struct wl_interface xdg_wm_base_interface;
#endif
#ifndef XDG_POSITIONER_INTERFACE
#define XDG_POSITIONER_INTERFACE
extern const struct wl_interface xdg_positioner_interface;
#endif
#ifndef XDG_SURFACE_INTERFACE
#define XDG_SURFACE_INTERFACE
extern const struct wl_interface xdg_surface_interface;
#endif
#ifndef XDG_TOPLEVEL_INTERFACE
#define XDG_TOPLEVEL_INTERFACE
extern const struct wl_interface xdg_toplevel_interface;
#endif
#ifndef XDG_POPUP_INTERFACE
#define XDG_POPUP_INTERFACE
extern const struct wl_interface xdg_popup_interface;
#endif
#ifndef XDG_WM_BASE_ERROR_ENUM
#define XDG_WM_BASE_ERROR_ENUM
enum xdg_wm_base_error {
  XDG_WM_BASE_ERROR_ROLE = 0,
  XDG_WM_BASE_ERROR_DEFUNCT_SURFACES = 1,
  XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP = 2,
  XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT = 3,
  XDG_WM_BASE_ERROR_INVALID_SURFACE_STATE = 4,
  XDG_WM_BASE_ERROR_INVALID_POSITIONER = 5,
  XDG_WM_BASE_ERROR_UNRESPONSIVE = 6,
};
#endif 
struct xdg_wm_base_listener {
  void (*ping)(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);
};
static inline int
xdg_wm_base_add_listener(struct xdg_wm_base *xdg_wm_base,
                         const struct xdg_wm_base_listener *listener,
                         void *data) {
  return wl_proxy_add_listener((struct wl_proxy *)xdg_wm_base,
                               (void (**)(void))listener, data);
}
#define XDG_WM_BASE_DESTROY 0
#define XDG_WM_BASE_CREATE_POSITIONER 1
#define XDG_WM_BASE_GET_XDG_SURFACE 2
#define XDG_WM_BASE_PONG 3
#define XDG_WM_BASE_PING_SINCE_VERSION 1
#define XDG_WM_BASE_DESTROY_SINCE_VERSION 1
#define XDG_WM_BASE_CREATE_POSITIONER_SINCE_VERSION 1
#define XDG_WM_BASE_GET_XDG_SURFACE_SINCE_VERSION 1
#define XDG_WM_BASE_PONG_SINCE_VERSION 1
static inline void xdg_wm_base_set_user_data(struct xdg_wm_base *xdg_wm_base,
                                             void *user_data) {
  wl_proxy_set_user_data((struct wl_proxy *)xdg_wm_base, user_data);
}
static inline void *xdg_wm_base_get_user_data(struct xdg_wm_base *xdg_wm_base) {
  return wl_proxy_get_user_data((struct wl_proxy *)xdg_wm_base);
}
static inline uint32_t
xdg_wm_base_get_version(struct xdg_wm_base *xdg_wm_base) {
  return wl_proxy_get_version((struct wl_proxy *)xdg_wm_base);
}
static inline void xdg_wm_base_destroy(struct xdg_wm_base *xdg_wm_base) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_wm_base, XDG_WM_BASE_DESTROY,
                         NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_wm_base),
                         WL_MARSHAL_FLAG_DESTROY);
}
static inline struct xdg_positioner *
xdg_wm_base_create_positioner(struct xdg_wm_base *xdg_wm_base) {
  struct wl_proxy *id;
  id = wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_wm_base, XDG_WM_BASE_CREATE_POSITIONER,
      &xdg_positioner_interface,
      wl_proxy_get_version((struct wl_proxy *)xdg_wm_base), 0, NULL);
  return (struct xdg_positioner *)id;
}
static inline struct xdg_surface *
xdg_wm_base_get_xdg_surface(struct xdg_wm_base *xdg_wm_base,
                            struct wl_surface *surface) {
  struct wl_proxy *id;
  id = wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_wm_base, XDG_WM_BASE_GET_XDG_SURFACE,
      &xdg_surface_interface,
      wl_proxy_get_version((struct wl_proxy *)xdg_wm_base), 0, NULL, surface);
  return (struct xdg_surface *)id;
}
static inline void xdg_wm_base_pong(struct xdg_wm_base *xdg_wm_base,
                                    uint32_t serial) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_wm_base, XDG_WM_BASE_PONG, NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_wm_base),
                         0, serial);
}
#ifndef XDG_POSITIONER_ERROR_ENUM
#define XDG_POSITIONER_ERROR_ENUM
enum xdg_positioner_error {
  XDG_POSITIONER_ERROR_INVALID_INPUT = 0,
};
#endif 
#ifndef XDG_POSITIONER_ANCHOR_ENUM
#define XDG_POSITIONER_ANCHOR_ENUM
enum xdg_positioner_anchor {
  XDG_POSITIONER_ANCHOR_NONE = 0,
  XDG_POSITIONER_ANCHOR_TOP = 1,
  XDG_POSITIONER_ANCHOR_BOTTOM = 2,
  XDG_POSITIONER_ANCHOR_LEFT = 3,
  XDG_POSITIONER_ANCHOR_RIGHT = 4,
  XDG_POSITIONER_ANCHOR_TOP_LEFT = 5,
  XDG_POSITIONER_ANCHOR_BOTTOM_LEFT = 6,
  XDG_POSITIONER_ANCHOR_TOP_RIGHT = 7,
  XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT = 8,
};
#endif 
#ifndef XDG_POSITIONER_GRAVITY_ENUM
#define XDG_POSITIONER_GRAVITY_ENUM
enum xdg_positioner_gravity {
  XDG_POSITIONER_GRAVITY_NONE = 0,
  XDG_POSITIONER_GRAVITY_TOP = 1,
  XDG_POSITIONER_GRAVITY_BOTTOM = 2,
  XDG_POSITIONER_GRAVITY_LEFT = 3,
  XDG_POSITIONER_GRAVITY_RIGHT = 4,
  XDG_POSITIONER_GRAVITY_TOP_LEFT = 5,
  XDG_POSITIONER_GRAVITY_BOTTOM_LEFT = 6,
  XDG_POSITIONER_GRAVITY_TOP_RIGHT = 7,
  XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT = 8,
};
#endif 
#ifndef XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_ENUM
#define XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_ENUM
enum xdg_positioner_constraint_adjustment {
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE = 0,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X = 1,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y = 2,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X = 4,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y = 8,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X = 16,
  XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y = 32,
};
#endif 
#define XDG_POSITIONER_DESTROY 0
#define XDG_POSITIONER_SET_SIZE 1
#define XDG_POSITIONER_SET_ANCHOR_RECT 2
#define XDG_POSITIONER_SET_ANCHOR 3
#define XDG_POSITIONER_SET_GRAVITY 4
#define XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT 5
#define XDG_POSITIONER_SET_OFFSET 6
#define XDG_POSITIONER_SET_REACTIVE 7
#define XDG_POSITIONER_SET_PARENT_SIZE 8
#define XDG_POSITIONER_SET_PARENT_CONFIGURE 9
#define XDG_POSITIONER_DESTROY_SINCE_VERSION 1
#define XDG_POSITIONER_SET_SIZE_SINCE_VERSION 1
#define XDG_POSITIONER_SET_ANCHOR_RECT_SINCE_VERSION 1
#define XDG_POSITIONER_SET_ANCHOR_SINCE_VERSION 1
#define XDG_POSITIONER_SET_GRAVITY_SINCE_VERSION 1
#define XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT_SINCE_VERSION 1
#define XDG_POSITIONER_SET_OFFSET_SINCE_VERSION 1
#define XDG_POSITIONER_SET_REACTIVE_SINCE_VERSION 3
#define XDG_POSITIONER_SET_PARENT_SIZE_SINCE_VERSION 3
#define XDG_POSITIONER_SET_PARENT_CONFIGURE_SINCE_VERSION 3
static inline void
xdg_positioner_set_user_data(struct xdg_positioner *xdg_positioner,
                             void *user_data) {
  wl_proxy_set_user_data((struct wl_proxy *)xdg_positioner, user_data);
}
static inline void *
xdg_positioner_get_user_data(struct xdg_positioner *xdg_positioner) {
  return wl_proxy_get_user_data((struct wl_proxy *)xdg_positioner);
}
static inline uint32_t
xdg_positioner_get_version(struct xdg_positioner *xdg_positioner) {
  return wl_proxy_get_version((struct wl_proxy *)xdg_positioner);
}
static inline void
xdg_positioner_destroy(struct xdg_positioner *xdg_positioner) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_DESTROY, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner),
      WL_MARSHAL_FLAG_DESTROY);
}
static inline void
xdg_positioner_set_size(struct xdg_positioner *xdg_positioner, int32_t width,
                        int32_t height) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_SIZE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, width,
      height);
}
static inline void
xdg_positioner_set_anchor_rect(struct xdg_positioner *xdg_positioner, int32_t x,
                               int32_t y, int32_t width, int32_t height) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_ANCHOR_RECT, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, x, y, width,
      height);
}
static inline void
xdg_positioner_set_anchor(struct xdg_positioner *xdg_positioner,
                          uint32_t anchor) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_ANCHOR, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, anchor);
}
static inline void
xdg_positioner_set_gravity(struct xdg_positioner *xdg_positioner,
                           uint32_t gravity) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_GRAVITY, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, gravity);
}
static inline void
xdg_positioner_set_constraint_adjustment(struct xdg_positioner *xdg_positioner,
                                         uint32_t constraint_adjustment) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner,
      XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0,
      constraint_adjustment);
}
static inline void
xdg_positioner_set_offset(struct xdg_positioner *xdg_positioner, int32_t x,
                          int32_t y) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_OFFSET, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, x, y);
}
static inline void
xdg_positioner_set_reactive(struct xdg_positioner *xdg_positioner) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_REACTIVE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0);
}
static inline void
xdg_positioner_set_parent_size(struct xdg_positioner *xdg_positioner,
                               int32_t parent_width, int32_t parent_height) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_PARENT_SIZE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, parent_width,
      parent_height);
}
static inline void
xdg_positioner_set_parent_configure(struct xdg_positioner *xdg_positioner,
                                    uint32_t serial) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_positioner, XDG_POSITIONER_SET_PARENT_CONFIGURE,
      NULL, wl_proxy_get_version((struct wl_proxy *)xdg_positioner), 0, serial);
}
#ifndef XDG_SURFACE_ERROR_ENUM
#define XDG_SURFACE_ERROR_ENUM
enum xdg_surface_error {
  XDG_SURFACE_ERROR_NOT_CONSTRUCTED = 1,
  XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED = 2,
  XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER = 3,
  XDG_SURFACE_ERROR_INVALID_SERIAL = 4,
  XDG_SURFACE_ERROR_INVALID_SIZE = 5,
  XDG_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT = 6,
};
#endif 
struct xdg_surface_listener {
  void (*configure)(void *data, struct xdg_surface *xdg_surface,
                    uint32_t serial);
};
static inline int
xdg_surface_add_listener(struct xdg_surface *xdg_surface,
                         const struct xdg_surface_listener *listener,
                         void *data) {
  return wl_proxy_add_listener((struct wl_proxy *)xdg_surface,
                               (void (**)(void))listener, data);
}
#define XDG_SURFACE_DESTROY 0
#define XDG_SURFACE_GET_TOPLEVEL 1
#define XDG_SURFACE_GET_POPUP 2
#define XDG_SURFACE_SET_WINDOW_GEOMETRY 3
#define XDG_SURFACE_ACK_CONFIGURE 4
#define XDG_SURFACE_CONFIGURE_SINCE_VERSION 1
#define XDG_SURFACE_DESTROY_SINCE_VERSION 1
#define XDG_SURFACE_GET_TOPLEVEL_SINCE_VERSION 1
#define XDG_SURFACE_GET_POPUP_SINCE_VERSION 1
#define XDG_SURFACE_SET_WINDOW_GEOMETRY_SINCE_VERSION 1
#define XDG_SURFACE_ACK_CONFIGURE_SINCE_VERSION 1
static inline void xdg_surface_set_user_data(struct xdg_surface *xdg_surface,
                                             void *user_data) {
  wl_proxy_set_user_data((struct wl_proxy *)xdg_surface, user_data);
}
static inline void *xdg_surface_get_user_data(struct xdg_surface *xdg_surface) {
  return wl_proxy_get_user_data((struct wl_proxy *)xdg_surface);
}
static inline uint32_t
xdg_surface_get_version(struct xdg_surface *xdg_surface) {
  return wl_proxy_get_version((struct wl_proxy *)xdg_surface);
}
static inline void xdg_surface_destroy(struct xdg_surface *xdg_surface) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_surface, XDG_SURFACE_DESTROY,
                         NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_surface),
                         WL_MARSHAL_FLAG_DESTROY);
}
static inline struct xdg_toplevel *
xdg_surface_get_toplevel(struct xdg_surface *xdg_surface) {
  struct wl_proxy *id;
  id = wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_surface, XDG_SURFACE_GET_TOPLEVEL,
      &xdg_toplevel_interface,
      wl_proxy_get_version((struct wl_proxy *)xdg_surface), 0, NULL);
  return (struct xdg_toplevel *)id;
}
static inline struct xdg_popup *
xdg_surface_get_popup(struct xdg_surface *xdg_surface,
                      struct xdg_surface *parent,
                      struct xdg_positioner *positioner) {
  struct wl_proxy *id;
  id = wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_surface, XDG_SURFACE_GET_POPUP,
      &xdg_popup_interface,
      wl_proxy_get_version((struct wl_proxy *)xdg_surface), 0, NULL, parent,
      positioner);
  return (struct xdg_popup *)id;
}
static inline void
xdg_surface_set_window_geometry(struct xdg_surface *xdg_surface, int32_t x,
                                int32_t y, int32_t width, int32_t height) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_surface,
                         XDG_SURFACE_SET_WINDOW_GEOMETRY, NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_surface),
                         0, x, y, width, height);
}
static inline void xdg_surface_ack_configure(struct xdg_surface *xdg_surface,
                                             uint32_t serial) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_surface, XDG_SURFACE_ACK_CONFIGURE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_surface), 0, serial);
}
#ifndef XDG_TOPLEVEL_ERROR_ENUM
#define XDG_TOPLEVEL_ERROR_ENUM
enum xdg_toplevel_error {
  XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE = 0,
  XDG_TOPLEVEL_ERROR_INVALID_PARENT = 1,
  XDG_TOPLEVEL_ERROR_INVALID_SIZE = 2,
};
#endif 
#ifndef XDG_TOPLEVEL_RESIZE_EDGE_ENUM
#define XDG_TOPLEVEL_RESIZE_EDGE_ENUM
enum xdg_toplevel_resize_edge {
  XDG_TOPLEVEL_RESIZE_EDGE_NONE = 0,
  XDG_TOPLEVEL_RESIZE_EDGE_TOP = 1,
  XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM = 2,
  XDG_TOPLEVEL_RESIZE_EDGE_LEFT = 4,
  XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT = 5,
  XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT = 6,
  XDG_TOPLEVEL_RESIZE_EDGE_RIGHT = 8,
  XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT = 9,
  XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};
#endif 
#ifndef XDG_TOPLEVEL_STATE_ENUM
#define XDG_TOPLEVEL_STATE_ENUM
enum xdg_toplevel_state {
  XDG_TOPLEVEL_STATE_MAXIMIZED = 1,
  XDG_TOPLEVEL_STATE_FULLSCREEN = 2,
  XDG_TOPLEVEL_STATE_RESIZING = 3,
  XDG_TOPLEVEL_STATE_ACTIVATED = 4,
  XDG_TOPLEVEL_STATE_TILED_LEFT = 5,
  XDG_TOPLEVEL_STATE_TILED_RIGHT = 6,
  XDG_TOPLEVEL_STATE_TILED_TOP = 7,
  XDG_TOPLEVEL_STATE_TILED_BOTTOM = 8,
  XDG_TOPLEVEL_STATE_SUSPENDED = 9,
};
#define XDG_TOPLEVEL_STATE_TILED_LEFT_SINCE_VERSION 2
#define XDG_TOPLEVEL_STATE_TILED_RIGHT_SINCE_VERSION 2
#define XDG_TOPLEVEL_STATE_TILED_TOP_SINCE_VERSION 2
#define XDG_TOPLEVEL_STATE_TILED_BOTTOM_SINCE_VERSION 2
#define XDG_TOPLEVEL_STATE_SUSPENDED_SINCE_VERSION 6
#endif 
#ifndef XDG_TOPLEVEL_WM_CAPABILITIES_ENUM
#define XDG_TOPLEVEL_WM_CAPABILITIES_ENUM
enum xdg_toplevel_wm_capabilities {
  XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU = 1,
  XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE = 2,
  XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN = 3,
  XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE = 4,
};
#endif 
struct xdg_toplevel_listener {
  void (*configure)(void *data, struct xdg_toplevel *xdg_toplevel,
                    int32_t width, int32_t height, struct wl_array *states);
  void (*close)(void *data, struct xdg_toplevel *xdg_toplevel);
  void (*configure_bounds)(void *data, struct xdg_toplevel *xdg_toplevel,
                           int32_t width, int32_t height);
  void (*wm_capabilities)(void *data, struct xdg_toplevel *xdg_toplevel,
                          struct wl_array *capabilities);
};
static inline int
xdg_toplevel_add_listener(struct xdg_toplevel *xdg_toplevel,
                          const struct xdg_toplevel_listener *listener,
                          void *data) {
  return wl_proxy_add_listener((struct wl_proxy *)xdg_toplevel,
                               (void (**)(void))listener, data);
}
#define XDG_TOPLEVEL_DESTROY 0
#define XDG_TOPLEVEL_SET_PARENT 1
#define XDG_TOPLEVEL_SET_TITLE 2
#define XDG_TOPLEVEL_SET_APP_ID 3
#define XDG_TOPLEVEL_SHOW_WINDOW_MENU 4
#define XDG_TOPLEVEL_MOVE 5
#define XDG_TOPLEVEL_RESIZE 6
#define XDG_TOPLEVEL_SET_MAX_SIZE 7
#define XDG_TOPLEVEL_SET_MIN_SIZE 8
#define XDG_TOPLEVEL_SET_MAXIMIZED 9
#define XDG_TOPLEVEL_UNSET_MAXIMIZED 10
#define XDG_TOPLEVEL_SET_FULLSCREEN 11
#define XDG_TOPLEVEL_UNSET_FULLSCREEN 12
#define XDG_TOPLEVEL_SET_MINIMIZED 13
#define XDG_TOPLEVEL_CONFIGURE_SINCE_VERSION 1
#define XDG_TOPLEVEL_CLOSE_SINCE_VERSION 1
#define XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION 4
#define XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION 5
#define XDG_TOPLEVEL_DESTROY_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_PARENT_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_TITLE_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_APP_ID_SINCE_VERSION 1
#define XDG_TOPLEVEL_SHOW_WINDOW_MENU_SINCE_VERSION 1
#define XDG_TOPLEVEL_MOVE_SINCE_VERSION 1
#define XDG_TOPLEVEL_RESIZE_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_MAX_SIZE_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_MIN_SIZE_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_MAXIMIZED_SINCE_VERSION 1
#define XDG_TOPLEVEL_UNSET_MAXIMIZED_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_FULLSCREEN_SINCE_VERSION 1
#define XDG_TOPLEVEL_UNSET_FULLSCREEN_SINCE_VERSION 1
#define XDG_TOPLEVEL_SET_MINIMIZED_SINCE_VERSION 1
static inline void xdg_toplevel_set_user_data(struct xdg_toplevel *xdg_toplevel,
                                              void *user_data) {
  wl_proxy_set_user_data((struct wl_proxy *)xdg_toplevel, user_data);
}
static inline void *
xdg_toplevel_get_user_data(struct xdg_toplevel *xdg_toplevel) {
  return wl_proxy_get_user_data((struct wl_proxy *)xdg_toplevel);
}
static inline uint32_t
xdg_toplevel_get_version(struct xdg_toplevel *xdg_toplevel) {
  return wl_proxy_get_version((struct wl_proxy *)xdg_toplevel);
}
static inline void xdg_toplevel_destroy(struct xdg_toplevel *xdg_toplevel) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_DESTROY,
                         NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_toplevel),
                         WL_MARSHAL_FLAG_DESTROY);
}
static inline void xdg_toplevel_set_parent(struct xdg_toplevel *xdg_toplevel,
                                           struct xdg_toplevel *parent) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_PARENT, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, parent);
}
static inline void xdg_toplevel_set_title(struct xdg_toplevel *xdg_toplevel,
                                          const char *title) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_TITLE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, title);
}
static inline void xdg_toplevel_set_app_id(struct xdg_toplevel *xdg_toplevel,
                                           const char *app_id) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_APP_ID, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, app_id);
}
static inline void
xdg_toplevel_show_window_menu(struct xdg_toplevel *xdg_toplevel,
                              struct wl_seat *seat, uint32_t serial, int32_t x,
                              int32_t y) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_toplevel,
                         XDG_TOPLEVEL_SHOW_WINDOW_MENU, NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_toplevel),
                         0, seat, serial, x, y);
}
static inline void xdg_toplevel_move(struct xdg_toplevel *xdg_toplevel,
                                     struct wl_seat *seat, uint32_t serial) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_MOVE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, seat, serial);
}
static inline void xdg_toplevel_resize(struct xdg_toplevel *xdg_toplevel,
                                       struct wl_seat *seat, uint32_t serial,
                                       uint32_t edges) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_RESIZE,
                         NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_toplevel),
                         0, seat, serial, edges);
}
static inline void xdg_toplevel_set_max_size(struct xdg_toplevel *xdg_toplevel,
                                             int32_t width, int32_t height) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_MAX_SIZE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, width, height);
}
static inline void xdg_toplevel_set_min_size(struct xdg_toplevel *xdg_toplevel,
                                             int32_t width, int32_t height) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_MIN_SIZE, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, width, height);
}
static inline void
xdg_toplevel_set_maximized(struct xdg_toplevel *xdg_toplevel) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_MAXIMIZED, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0);
}
static inline void
xdg_toplevel_unset_maximized(struct xdg_toplevel *xdg_toplevel) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_UNSET_MAXIMIZED, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0);
}
static inline void
xdg_toplevel_set_fullscreen(struct xdg_toplevel *xdg_toplevel,
                            struct wl_output *output) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_FULLSCREEN, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0, output);
}
static inline void
xdg_toplevel_unset_fullscreen(struct xdg_toplevel *xdg_toplevel) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_UNSET_FULLSCREEN, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0);
}
static inline void
xdg_toplevel_set_minimized(struct xdg_toplevel *xdg_toplevel) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_toplevel, XDG_TOPLEVEL_SET_MINIMIZED, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_toplevel), 0);
}
#ifndef XDG_POPUP_ERROR_ENUM
#define XDG_POPUP_ERROR_ENUM
enum xdg_popup_error {
  XDG_POPUP_ERROR_INVALID_GRAB = 0,
};
#endif 
struct xdg_popup_listener {
  void (*configure)(void *data, struct xdg_popup *xdg_popup, int32_t x,
                    int32_t y, int32_t width, int32_t height);
  void (*popup_done)(void *data, struct xdg_popup *xdg_popup);
  void (*repositioned)(void *data, struct xdg_popup *xdg_popup, uint32_t token);
};
static inline int
xdg_popup_add_listener(struct xdg_popup *xdg_popup,
                       const struct xdg_popup_listener *listener, void *data) {
  return wl_proxy_add_listener((struct wl_proxy *)xdg_popup,
                               (void (**)(void))listener, data);
}
#define XDG_POPUP_DESTROY 0
#define XDG_POPUP_GRAB 1
#define XDG_POPUP_REPOSITION 2
#define XDG_POPUP_CONFIGURE_SINCE_VERSION 1
#define XDG_POPUP_POPUP_DONE_SINCE_VERSION 1
#define XDG_POPUP_REPOSITIONED_SINCE_VERSION 3
#define XDG_POPUP_DESTROY_SINCE_VERSION 1
#define XDG_POPUP_GRAB_SINCE_VERSION 1
#define XDG_POPUP_REPOSITION_SINCE_VERSION 3
static inline void xdg_popup_set_user_data(struct xdg_popup *xdg_popup,
                                           void *user_data) {
  wl_proxy_set_user_data((struct wl_proxy *)xdg_popup, user_data);
}
static inline void *xdg_popup_get_user_data(struct xdg_popup *xdg_popup) {
  return wl_proxy_get_user_data((struct wl_proxy *)xdg_popup);
}
static inline uint32_t xdg_popup_get_version(struct xdg_popup *xdg_popup) {
  return wl_proxy_get_version((struct wl_proxy *)xdg_popup);
}
static inline void xdg_popup_destroy(struct xdg_popup *xdg_popup) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_popup, XDG_POPUP_DESTROY, NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_popup),
                         WL_MARSHAL_FLAG_DESTROY);
}
static inline void xdg_popup_grab(struct xdg_popup *xdg_popup,
                                  struct wl_seat *seat, uint32_t serial) {
  wl_proxy_marshal_flags((struct wl_proxy *)xdg_popup, XDG_POPUP_GRAB, NULL,
                         wl_proxy_get_version((struct wl_proxy *)xdg_popup), 0,
                         seat, serial);
}
static inline void xdg_popup_reposition(struct xdg_popup *xdg_popup,
                                        struct xdg_positioner *positioner,
                                        uint32_t token) {
  wl_proxy_marshal_flags(
      (struct wl_proxy *)xdg_popup, XDG_POPUP_REPOSITION, NULL,
      wl_proxy_get_version((struct wl_proxy *)xdg_popup), 0, positioner, token);
}


#ifdef SWCL_IMPLEMENTATION

// ------------------------------------------------------------------------- //
//                                                                           //
//                     XDG_SHELL_PROTOCOL IMPLEMENTATION                     //
//                        INSERTED DURING BUILD STEP                         //
//                                                                           //
// ------------------------------------------------------------------------- //

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"
#ifndef __has_attribute
# define __has_attribute(x) 0  
#endif
#if (__has_attribute(visibility) || defined(__GNUC__) && __GNUC__ >= 4)
#define WL_PRIVATE __attribute__ ((visibility("hidden")))
#else
#define WL_PRIVATE
#endif
extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface xdg_popup_interface;
extern const struct wl_interface xdg_positioner_interface;
extern const struct wl_interface xdg_surface_interface;
extern const struct wl_interface xdg_toplevel_interface;
static const struct wl_interface *xdg_shell_types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	&xdg_positioner_interface,
	&xdg_surface_interface,
	&wl_surface_interface,
	&xdg_toplevel_interface,
	&xdg_popup_interface,
	&xdg_surface_interface,
	&xdg_positioner_interface,
	&xdg_toplevel_interface,
	&wl_seat_interface,
	NULL,
	NULL,
	NULL,
	&wl_seat_interface,
	NULL,
	&wl_seat_interface,
	NULL,
	NULL,
	&wl_output_interface,
	&wl_seat_interface,
	NULL,
	&xdg_positioner_interface,
	NULL,
};
static const struct wl_message xdg_wm_base_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "create_positioner", "n", xdg_shell_types + 4 },
	{ "get_xdg_surface", "no", xdg_shell_types + 5 },
	{ "pong", "u", xdg_shell_types + 0 },
};
static const struct wl_message xdg_wm_base_events[] = {
	{ "ping", "u", xdg_shell_types + 0 },
};
WL_PRIVATE const struct wl_interface xdg_wm_base_interface = {
	"xdg_wm_base", 6,
	4, xdg_wm_base_requests,
	1, xdg_wm_base_events,
};
static const struct wl_message xdg_positioner_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "set_size", "ii", xdg_shell_types + 0 },
	{ "set_anchor_rect", "iiii", xdg_shell_types + 0 },
	{ "set_anchor", "u", xdg_shell_types + 0 },
	{ "set_gravity", "u", xdg_shell_types + 0 },
	{ "set_constraint_adjustment", "u", xdg_shell_types + 0 },
	{ "set_offset", "ii", xdg_shell_types + 0 },
	{ "set_reactive", "3", xdg_shell_types + 0 },
	{ "set_parent_size", "3ii", xdg_shell_types + 0 },
	{ "set_parent_configure", "3u", xdg_shell_types + 0 },
};
WL_PRIVATE const struct wl_interface xdg_positioner_interface = {
	"xdg_positioner", 6,
	10, xdg_positioner_requests,
	0, NULL,
};
static const struct wl_message xdg_surface_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "get_toplevel", "n", xdg_shell_types + 7 },
	{ "get_popup", "n?oo", xdg_shell_types + 8 },
	{ "set_window_geometry", "iiii", xdg_shell_types + 0 },
	{ "ack_configure", "u", xdg_shell_types + 0 },
};
static const struct wl_message xdg_surface_events[] = {
	{ "configure", "u", xdg_shell_types + 0 },
};
WL_PRIVATE const struct wl_interface xdg_surface_interface = {
	"xdg_surface", 6,
	5, xdg_surface_requests,
	1, xdg_surface_events,
};
static const struct wl_message xdg_toplevel_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "set_parent", "?o", xdg_shell_types + 11 },
	{ "set_title", "s", xdg_shell_types + 0 },
	{ "set_app_id", "s", xdg_shell_types + 0 },
	{ "show_window_menu", "ouii", xdg_shell_types + 12 },
	{ "move", "ou", xdg_shell_types + 16 },
	{ "resize", "ouu", xdg_shell_types + 18 },
	{ "set_max_size", "ii", xdg_shell_types + 0 },
	{ "set_min_size", "ii", xdg_shell_types + 0 },
	{ "set_maximized", "", xdg_shell_types + 0 },
	{ "unset_maximized", "", xdg_shell_types + 0 },
	{ "set_fullscreen", "?o", xdg_shell_types + 21 },
	{ "unset_fullscreen", "", xdg_shell_types + 0 },
	{ "set_minimized", "", xdg_shell_types + 0 },
};
static const struct wl_message xdg_toplevel_events[] = {
	{ "configure", "iia", xdg_shell_types + 0 },
	{ "close", "", xdg_shell_types + 0 },
	{ "configure_bounds", "4ii", xdg_shell_types + 0 },
	{ "wm_capabilities", "5a", xdg_shell_types + 0 },
};
WL_PRIVATE const struct wl_interface xdg_toplevel_interface = {
	"xdg_toplevel", 6,
	14, xdg_toplevel_requests,
	4, xdg_toplevel_events,
};
static const struct wl_message xdg_popup_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "grab", "ou", xdg_shell_types + 22 },
	{ "reposition", "3ou", xdg_shell_types + 24 },
};
static const struct wl_message xdg_popup_events[] = {
	{ "configure", "iiii", xdg_shell_types + 0 },
	{ "popup_done", "", xdg_shell_types + 0 },
	{ "repositioned", "3u", xdg_shell_types + 0 },
};
WL_PRIVATE const struct wl_interface xdg_popup_interface = {
	"xdg_popup", 6,
	3, xdg_popup_requests,
	3, xdg_popup_events,
};


// ------------------------------------------------------------------------- //
//                                                                           //
//                         APPLICATION IMPLEMENTATION                        //
//                                                                           //
// ------------------------------------------------------------------------- //

// -------- xdg_wm_base events callbacks -------- //

static inline void on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base,
                                       uint32_t serial) {
  xdg_wm_base_pong(wm_base, serial);
}

static inline void on_wl_seat_name(void *data, struct wl_seat *seat,
                                   const char *name) {}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = on_xdg_wm_base_ping,
};

// -------- wl_pointer events callbacks -------- //

static inline void on_wl_pointer_button(void *data, struct wl_pointer *pointer,
                                        uint32_t serial, uint32_t time,
                                        uint32_t button, uint32_t state) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->wl_pointer_serial = serial;
  if (app->on_mouse_button_cb)
    app->on_mouse_button_cb(app->current_window, (SWCLMouseButton)button,
                            (SWCLButtonState)state);
}

static inline void on_wl_pointer_enter(void *data, struct wl_pointer *pointer,
                                       uint32_t serial,
                                       struct wl_surface *surface, wl_fixed_t x,
                                       wl_fixed_t y) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->wl_pointer_serial = serial;
  app->cursor_pos.x = wl_fixed_to_int(x);
  app->cursor_pos.y = wl_fixed_to_int(y);
  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = (SWCLWindow *)app->windows.items[i];
    if (win->wl_surface == surface) {
      app->current_window = win;
      if (app->on_pointer_enter_cb)
        app->on_pointer_enter_cb(app->current_window, app->cursor_pos.x,
                                 app->cursor_pos.y);
    }
  }
};

static inline void on_wl_pointer_leave(void *data, struct wl_pointer *pointer,
                                       uint32_t serial,
                                       struct wl_surface *surface) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->on_pointer_leave_cb)
    app->on_pointer_leave_cb(app->current_window);
  app->wl_pointer_serial = serial;
};

static inline void on_wl_pointer_motion(void *data, struct wl_pointer *pointer,
                                        uint32_t time, wl_fixed_t x,
                                        wl_fixed_t y) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->cursor_pos.x = wl_fixed_to_int(x);
  app->cursor_pos.y = wl_fixed_to_int(y);
  if (app->on_pointer_motion_cb)
    app->on_pointer_motion_cb(app->current_window, app->cursor_pos.x,
                              app->cursor_pos.y);
};

static inline void on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                                      uint32_t time, uint32_t axis,
                                      wl_fixed_t value) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->on_mouse_scroll_cb)
    app->on_mouse_scroll_cb(app->current_window,
                            (SWCLScrollDirection)(1 ? value > 0 : 0));
};

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = on_wl_pointer_enter,
    .leave = on_wl_pointer_leave,
    .motion = on_wl_pointer_motion,
    .button = on_wl_pointer_button,
    .axis = on_wl_pointer_axis,
};

// -------- wl_keyboard events callbacks -------- //

static inline void on_wl_kb_keymap(void *data, struct wl_keyboard *wl_keyboard,
                                   uint32_t format, int32_t fd, uint32_t size) {
}

static inline void on_wl_kb_focus_enter_surface(void *data,
                                                struct wl_keyboard *wl_keyboard,
                                                uint32_t serial,
                                                struct wl_surface *surface,
                                                struct wl_array *key) {}

static inline void on_wl_kb_focus_leave_surface(void *data,
                                                struct wl_keyboard *wl_keyboard,
                                                uint32_t serial,
                                                struct wl_surface *surface) {}

static inline void on_wl_kb_key(void *data, struct wl_keyboard *wl_keyboard,
                                uint32_t serial, uint32_t time, uint32_t key,
                                uint32_t state) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->on_keyboard_key_cb)
    app->on_keyboard_key_cb(app->current_window, key, (SWCLButtonState)state);
}

static inline void on_wl_kb_mod(void *data, struct wl_keyboard *wl_keyboard,
                                uint32_t serial, uint32_t mods_depressed,
                                uint32_t mods_latched, uint32_t mods_locked,
                                uint32_t group) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->on_keyboard_mod_key_cb)
    app->on_keyboard_mod_key_cb(app->current_window, mods_depressed,
                                mods_latched, mods_locked, group);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = on_wl_kb_keymap,
    .enter = on_wl_kb_focus_enter_surface,
    .leave = on_wl_kb_focus_leave_surface,
    .key = on_wl_kb_key,
    .modifiers = on_wl_kb_mod,
};

// -------- wl_seat events callbacks -------- //

static inline void on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                           uint32_t capability) {
  SWCLApplication *app = (SWCLApplication *)data;

  if ((capability & WL_SEAT_CAPABILITY_POINTER) && !app->wl_pointer) {
    app->wl_pointer = wl_seat_get_pointer(seat);
    if (app->wl_pointer) {
      SWCL_LOG_DEBUG("Got pointer");
      wl_pointer_add_listener(app->wl_pointer, &wl_pointer_listener, app);
      wl_display_roundtrip(app->wl_display);
    } else {
      SWCL_LOG_DEBUG("No pointer found");
    }
  }

  if ((capability & WL_SEAT_CAPABILITY_KEYBOARD) && !app->wl_keyboard) {
    app->wl_keyboard = wl_seat_get_keyboard(seat);
    if (app->wl_keyboard) {
      SWCL_LOG_DEBUG("Got keyboard");
      wl_keyboard_add_listener(app->wl_keyboard, &wl_keyboard_listener, app);
      wl_display_roundtrip(app->wl_display);
    } else {
      SWCL_LOG_DEBUG("No keyboard found");
    }
  }
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = on_wl_seat_capabilities,
    .name = on_wl_seat_name,
};

// -------- wl_registry events callbacks -------- //

static inline void on_wl_registry_global(void *data,
                                         struct wl_registry *registry,
                                         uint32_t id, const char *interface,
                                         uint32_t version) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    app->wl_compositor = (struct wl_compositor *)wl_registry_bind(
        registry, id, &wl_compositor_interface, 1);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    app->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
        registry, id, &xdg_wm_base_interface, 2);
    xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, NULL);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 2);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    app->wl_seat =
        (struct wl_seat *)wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    app->wl_cursor_shm =
        (struct wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, 1);
  }
  // else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
  //   app->wlr_layer_shell =
  //       wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
  //   SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  // }
}

static inline void on_wl_registry_global_remove(void *data,
                                                struct wl_registry *registry,
                                                uint32_t id) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = on_wl_registry_global,
    .global_remove = on_wl_registry_global_remove,
};

// ---------- APPLICATION METHODS ---------- //

static inline SWCLApplication *swcl_application_new(SWCLConfig *cfg) {
  SWCLApplication *app = SWCL_ALLOC(SWCLApplication);

  app->app_id = cfg->app_id;
  app->running = false;

  // Set callbacks
  app->on_pointer_enter_cb = cfg->on_pointer_enter_cb;
  app->on_pointer_leave_cb = cfg->on_pointer_leave_cb;
  app->on_pointer_motion_cb = cfg->on_pointer_motion_cb;
  app->on_mouse_button_cb = cfg->on_mouse_button_cb;
  app->on_mouse_scroll_cb = cfg->on_mouse_scroll_cb;
  app->on_keyboard_key_cb = cfg->on_keyboard_key_cb;
  app->on_keyboard_mod_key_cb = cfg->on_keyboard_mod_key_cb;

  app->wl_display = wl_display_connect(NULL);
  if (!app->wl_display)
    SWCL_PANIC("Failed to connect to wl_display");
  else
    SWCL_LOG_DEBUG("Connected to wl_display");

  app->wl_registry = wl_display_get_registry(app->wl_display);
  if (!app->wl_registry)
    SWCL_PANIC("Failed to connect to wl_registry");
  else
    SWCL_LOG_DEBUG("Connected to wl_registry");

  wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
  wl_display_roundtrip(app->wl_display);

  // Get EGLDisplay
  app->egl_display = eglGetDisplay(app->wl_display);
  if (app->egl_display == EGL_NO_DISPLAY)
    SWCL_PANIC("Failed to get EGLDisplay");
  else
    SWCL_LOG_DEBUG("Got EGLDisplay");

  // Init EGL
  EGLint major, minor;
  if (!eglInitialize(app->egl_display, &major, &minor))
    SWCL_PANIC("Failed to init EGL");
  else
    SWCL_LOG_DEBUG("Initialized EGL");

  // Bind OpenGL ES API to EGL
  if (!eglBindAPI(EGL_OPENGL_API))
    SWCL_PANIC("Failed to bind OpenGL to EGL");
  else
    SWCL_LOG_DEBUG("Binded OpenGL to EGL");

  const EGLint config_attrs[] = {
      EGL_SURFACE_TYPE,
      EGL_WINDOW_BIT,
      EGL_RED_SIZE,
      8,
      EGL_GREEN_SIZE,
      8,
      EGL_BLUE_SIZE,
      8,
      EGL_ALPHA_SIZE,
      8,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_BIT,
      EGL_SAMPLE_BUFFERS,
      1,
      EGL_SAMPLES,
      4,
      EGL_NONE,
  };

  // Choose config
  EGLint num;
  if (eglChooseConfig(app->egl_display, config_attrs, &app->egl_config, 1,
                      &num) == EGL_FALSE ||
      num == 0) {
    SWCL_PANIC("Failed to choose EGL config");
  } else
    SWCL_LOG_DEBUG("Chosen EGL config");

  // Create EGL context
  const EGLint context_attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  app->egl_context = eglCreateContext(app->egl_display, app->egl_config,
                                      EGL_NO_CONTEXT, context_attrs);
  if (!app->egl_context) {
    SWCL_PANIC("Failed to create EGL context");
  } else
    SWCL_LOG_DEBUG("Created EGL context");

  glEnable(GL_MULTISAMPLE);

  app->windows = swcl_array_new(2);

  return app;
}

static inline void swcl_application_run(SWCLApplication *app) {
  app->running = true;
  while (app->running) {
    wl_display_dispatch(app->wl_display);
  }
  // Cleanup
  swcl_array_free(app->windows);
  wl_display_disconnect(app->wl_display);
  free(app);
}

static inline void swcl_application_quit(SWCLApplication *app) {
  app->running = false;
}

static inline void swcl_application_set_cursor(SWCLApplication *app,
                                               const char *name, uint8_t size) {
  if (!app->current_cursor_name)
    goto create;
  if (!strcmp(app->current_cursor_name, name))
    goto update;

create:
  app->wl_cursor_surface = wl_compositor_create_surface(app->wl_compositor);
  app->wl_cursor_theme = wl_cursor_theme_load(NULL, size, app->wl_cursor_shm);
  goto update;

update:
  app->wl_cursor = wl_cursor_theme_get_cursor(app->wl_cursor_theme, name);
  app->wl_cursor_image = app->wl_cursor->images[0];
  app->wl_cursor_buffer = wl_cursor_image_get_buffer(app->wl_cursor_image);
  wl_surface_attach(app->wl_cursor_surface, app->wl_cursor_buffer, 0, 0);
  goto finish;

finish:
  wl_surface_damage(app->wl_cursor_surface, 0, 0, app->wl_cursor_image->width,
                    app->wl_cursor_image->height);
  wl_surface_commit(app->wl_cursor_surface);
  wl_pointer_set_cursor(app->wl_pointer, app->wl_pointer_serial,
                        app->wl_cursor_surface, app->wl_cursor_image->hotspot_x,
                        app->wl_cursor_image->hotspot_y);
  app->current_cursor_name = (char *)name;
}

// ------------------------------------------------------------------------- //
//                                                                           //
//                            WINDOW IMPLEMENTATION                          //
//                                                                           //
// ------------------------------------------------------------------------- //

static void __swcl_window_make_current(SWCLWindow *win);

// -------- xdg_toplevel events callbacks -------- //

static inline void on_xdg_toplevel_configure(void *data,
                                             struct xdg_toplevel *toplevel,
                                             int32_t width, int32_t height,
                                             struct wl_array *states) {
  if (width == 0 || height == 0)
    return;
  SWCL_LOG_DEBUG("xdg_toplevel configure. width=%d, height=%d", width, height);
  SWCLWindow *win = (SWCLWindow *)data;

  // Loop over the states array
  uint32_t *state;
  for (state = (uint32_t *)(states)->data;
       (const char *)state < ((const char *)(states)->data + (states)->size);
       (state)++) {
    switch (*state) {
    // Set maximized state
    case XDG_TOPLEVEL_STATE_MAXIMIZED:
      win->maximized = true;
    // Set unmaximized state
    case XDG_TOPLEVEL_STATE_ACTIVATED:
      if (win->maximized && (win->width > width || win->height > height)) {
        win->maximized = false;
      }
    }
  }

  // Resize window if needed
  if (win->egl_window && win->width != width || win->height != height) {
    win->width = width;
    win->height = height;
    glViewport(0, 0, width, height);
    wl_egl_window_resize(win->egl_window, width, height, 0, 0);
  }
}

static inline void on_xdg_toplevel_close(void *data,
                                         struct xdg_toplevel *toplevel) {
  SWCLWindow *win = (SWCLWindow *)data;
  win->app->running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = on_xdg_toplevel_configure,
    .close = on_xdg_toplevel_close,
};

// -------- xdg_surface events callbacks -------- //

static inline void on_xdg_surface_configure(void *data,
                                            struct xdg_surface *surface,
                                            uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = on_xdg_surface_configure,
};

// -------- wl_callback events callback -------- //

static void on_new_frame(void *data, struct wl_callback *cb, uint32_t cb_data);
static const struct wl_callback_listener wl_callback_listener = {
    .done = on_new_frame,
};

static inline void on_new_frame(void *data, struct wl_callback *cb,
                                uint32_t cb_data) {
  SWCLWindow *win = (SWCLWindow *)data;
  __swcl_window_make_current(win);
  wl_callback_destroy(cb);
  cb = wl_surface_frame(win->wl_surface);
  wl_callback_add_listener(cb, &wl_callback_listener, win);
  win->on_draw_cb(win);
}

// ---------- WINDOW METHODS ---------- //

static inline SWCLWindow *swcl_window_new(SWCLApplication *app, char *title,
                                          uint16_t width, uint16_t height,
                                          uint16_t min_width,
                                          uint16_t min_height, bool maximized,
                                          bool fullscreen,
                                          void (*draw_func)(SWCLWindow *win)) {

  SWCLWindow *win = SWCL_ALLOC(SWCLWindow);

  win->id = swcl_generate_id();
  win->width = width;
  win->height = height;
  win->min_width = min_width;
  win->min_height = min_height;
  win->maximized = maximized;
  win->fullscreen = fullscreen;
  win->title = title;
  win->on_draw_cb = draw_func;
  win->app = app;

  SWCL_LOG_DEBUG("Create new window with id: %d, width: %d, height: %d",
                 win->id, win->width, win->height);

  // Get wl_surface
  if (!app->wl_compositor)
    SWCL_PANIC("Failed to get wl_compositor");

  win->wl_surface = wl_compositor_create_surface(app->wl_compositor);
  if (!win->wl_surface)
    SWCL_PANIC("Failed to get wl_surface");
  else
    SWCL_LOG_DEBUG("Got wl_surface");

  // Get wl_callback for surface frame
  win->wl_callback = wl_surface_frame(win->wl_surface);
  if (!win->wl_callback)
    SWCL_PANIC("Failed to get wl_callback");
  else {
    SWCL_LOG_DEBUG("Got wl_callback");
    wl_callback_add_listener(win->wl_callback, &wl_callback_listener, win);
    wl_display_roundtrip(app->wl_display);
  }

  // Get xdg_surface
  win->xdg_surface =
      xdg_wm_base_get_xdg_surface(app->xdg_wm_base, win->wl_surface);
  if (!win->xdg_surface)
    SWCL_PANIC("Failed to get xdg_surface");
  else {
    SWCL_LOG_DEBUG("Got xdg_surface");
    xdg_surface_add_listener(win->xdg_surface, &xdg_surface_listener, NULL);
  }

  // Get xdg_toplevel
  win->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
  if (!win->xdg_toplevel)
    SWCL_PANIC("Failed to get xdg_toplevel");
  else {
    SWCL_LOG_DEBUG("Got xdg_toplevel");
    xdg_toplevel_add_listener(win->xdg_toplevel, &xdg_toplevel_listener, win);
    wl_display_roundtrip(app->wl_display);
  }

  // Create EGL window
  win->egl_window =
      wl_egl_window_create(win->wl_surface, win->width, win->height);

  // Create EGL surface
  win->egl_surface =
      eglCreateWindowSurface(app->egl_display, app->egl_config,
                             (EGLNativeWindowType)win->egl_window, NULL);
  if (win->egl_surface == EGL_NO_SURFACE)
    SWCL_PANIC("Failed to create EGL surface");
  else
    SWCL_LOG_DEBUG("Created EGL surface");

  // Setup xdg_toplevel
  xdg_toplevel_set_app_id(win->xdg_toplevel, app->app_id);
  if (win->title)
    xdg_toplevel_set_title(win->xdg_toplevel, win->title);
  if (win->min_height && win->min_width)
    xdg_toplevel_set_min_size(win->xdg_toplevel, win->min_width,
                              win->min_height);
  if (win->maximized)
    xdg_toplevel_set_maximized(win->xdg_toplevel);
  else
    xdg_toplevel_unset_maximized(win->xdg_toplevel);

  if (win->fullscreen)
    xdg_toplevel_set_fullscreen(win->xdg_toplevel, NULL);
  else
    xdg_toplevel_unset_fullscreen(win->xdg_toplevel);

  swcl_array_append(&app->windows, win);
  SWCL_LOG_DEBUG("Created window with id=%d, at %p", win->id, win);
  return win;
}

static inline void swcl_window_show(SWCLWindow *win) {
  __swcl_window_make_current(win);
  win->on_draw_cb(win);
}

static inline void __swcl_window_make_current(SWCLWindow *win) {
  if (eglMakeCurrent(win->app->egl_display, win->egl_surface, win->egl_surface,
                     win->app->egl_context) == EGL_FALSE)
    SWCL_PANIC("Failed to make egl_surface current");
}

static inline void swcl_window_drag(SWCLWindow *win) {
  xdg_toplevel_move(win->xdg_toplevel, win->app->wl_seat,
                    win->app->wl_pointer_serial);
}

static inline void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge) {
  xdg_toplevel_resize(win->xdg_toplevel, win->app->wl_seat,
                      win->app->wl_pointer_serial, edge);
}

static inline void swcl_window_swap_buffers(SWCLWindow *win) {
  eglSwapBuffers(win->app->egl_display, win->egl_surface);
}

static inline void swcl_window_set_title(SWCLWindow *win, char *title) {
  win->title = title;
  xdg_toplevel_set_title(win->xdg_toplevel, title);
}

static inline void swcl_window_set_maximized(SWCLWindow *win, bool maximized) {
  win->maximized = maximized;
  if (maximized)
    xdg_toplevel_set_maximized(win->xdg_toplevel);
  else
    xdg_toplevel_unset_maximized(win->xdg_toplevel);
}

static inline void swcl_window_minimize(SWCLWindow *win) {
  xdg_toplevel_set_minimized(win->xdg_toplevel);
}

static inline void swcl_window_set_min_size(SWCLWindow *win, int min_width,
                                            int min_height) {
  if (min_width <= 0 || min_height <= 0)
    return;
  win->min_width = min_width;
  win->min_height = min_height;
  xdg_toplevel_set_min_size(win->xdg_toplevel, min_width, min_height);
}

// inline void swcl_window_ancor(SWCLWindow *win, SWCLAncor ancor) {
//   SWCL_NOT_IMPLEMENTED("swcl_window_ancor");
//   return;
//   if (!win->app->zwlr_layer_shell) {
//     SWCL_LOG("Compositor is not supporting wlr_layer_shell");
//     return;
//   }
// }

// inline void swcl_window_request_ssr(SWCLWindow *win) {
//   SWCL_NOT_IMPLEMENTED("swcl_window_request_ssr");
//   return;
// }

// ------------------------------------------------------------------------- //
//                                                                           //
//                           DRAWING IMPLEMENTATION                          //
//                                                                           //
// ------------------------------------------------------------------------- //

// Setup coordinates system with top left corner as 0. If window size is not
// changed - do nothing.
static inline void __swcl_set_orthographic_projection() {
  uint32_t width;
  uint32_t height;
  GLint viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  if (width && height && viewport[2] == width && viewport[3] == height)
    return;

  width = viewport[2];
  height = viewport[3];

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static inline void swcl_clear_background(SWCLColor color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

// Draw rectangle with given color and dimentions.
static inline void swcl_draw_rect(SWCLColor color, SWCLRect rect) {
  if (color.a < 255) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  __swcl_set_orthographic_projection();
  glColor4ub(color.r, color.g, color.b, color.a);
  glBegin(GL_QUADS);
  glVertex2i(rect.x, rect.y);
  glVertex2i(rect.x + rect.w, rect.y);
  glVertex2i(rect.x + rect.w, rect.y + rect.h);
  glVertex2i(rect.x, rect.y + rect.h);
  glEnd();
  if (color.a < 255)
    glDisable(GL_BLEND);
}

static inline void swcl_draw_rounded_rect(SWCLColor color, SWCLRect rect,
                                          int radius) {
  if (radius == 0) {
    swcl_draw_rect(color, rect);
    return;
  }
  // Draw corner circles
  swcl_draw_circle(
      color, (SWCLCircle){rect.x + radius, rect.y + radius, (uint32_t)radius});
  swcl_draw_circle(color, (SWCLCircle){rect.x + rect.w - radius,
                                       rect.y + radius, (uint32_t)radius});
  swcl_draw_circle(color,
                   (SWCLCircle){rect.x + radius, rect.y + rect.h - radius,
                                (uint32_t)radius});
  swcl_draw_circle(color,
                   (SWCLCircle){rect.x + rect.w - radius,
                                rect.y + rect.h - radius, (uint32_t)radius});
  // Draw rects
  swcl_draw_rect(
      color, (SWCLRect){rect.x + radius, rect.y, rect.w - radius * 2, rect.h});
  swcl_draw_rect(
      color, (SWCLRect){rect.x, rect.y + radius, rect.w, rect.h - radius * 2});
}

static inline void swcl_draw_circle(SWCLColor color, SWCLCircle circle) {
  if (color.a < 255) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  __swcl_set_orthographic_projection();
  glColor4ub(color.r, color.g, color.b, color.a);
  glBegin(GL_POLYGON);
  for (int i = 0; i <= 100; i++) {
    double angle = 2 * M_PI * i / 100;
    double x = cos(angle) * circle.r;
    double y = sin(angle) * circle.r;
    glVertex2d(circle.cx + x, circle.cy + y);
  }
  glEnd();
  if (color.a < 255)
    glDisable(GL_BLEND);
}

#endif // SWCL_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // SWCL_H
