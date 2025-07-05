// Copyright 2024 Vlad Krupinskii <mrvladus@yandex.ru>
// SPDX-License-Identifier: MIT

// SWCL - Simple Wayland Client Library
// Create Wayland clients easily.
// This header-only library simplifies creating native Wayland window, receiving
// mouse and keyboard events, natively drag and resize the window.

// Linker flags: -lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm

#ifndef SWCL_H
#define SWCL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------- ENUMS ---------- //

// Direction of the mouse scroll wheel
typedef enum {
  SWCL_BUTTON_RELEASED = 0,
  SWCL_BUTTON_PRESSED = 1,
} SWCLButtonState;

// Direction of the mouse scroll wheel
typedef enum {
  SWCL_SCROLL_UP = 0,
  SWCL_SCROLL_DOWN = 1,
} SWCLScrollDirection;

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

// Mouse buttons keycodes
typedef enum {
  SWCL_MOUSE_1 = 272, // Left
  SWCL_MOUSE_2 = 273, // Right
  SWCL_MOUSE_3 = 274, // Middle
  SWCL_MOUSE_4 = 275, // Back
  SWCL_MOUSE_5 = 276, // Forward
} SWCLMouseButton;

// ---------- STRUCTS ---------- //

// Window type.
typedef struct SWCLWindow SWCLWindow;

// Application type.
typedef struct SWCLApplication SWCLApplication;

// ---------- APPLICATION ---------- //

// Initialize SWCL.
// This will initialize connection to Wayland display, register mouse, keyboard
// and touch devices. Also it will initialize EGL and create OpenGL context.
//
// Arguments:
// app_id: a string in format "com.example.App".
// pointer_enter_cb: callback for the event of mouse pointer entering window.
// pointer_motion_cb: callback for the event of mouse pointer motion over the window.
// pointer_leave_cb: callback for the event of mouse pointer leaving window.
// mouse_button_cb: callback for the event of mouse button press or release.
// mouse_scroll_cb: callback for the event of scrolling over window.
// keyboard_key_cb: callback for the event of keyboard key press or release.
// keyboard_mod_key_cb: callback for the event of keyboard modifier key press or release.
SWCLApplication *
swcl_application_new(const char *app_id, void (*pointer_enter_cb)(SWCLWindow *win, size_t x, size_t y),
                     void (*pointer_motion_cb)(SWCLWindow *win, size_t x, size_t y),
                     void (*pointer_leave_cb)(SWCLWindow *win),
                     void (*mouse_button_cb)(SWCLWindow *win, SWCLMouseButton btn, SWCLButtonState state),
                     void (*mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir),
                     void (*keyboard_key_cb)(SWCLWindow *win, size_t key, SWCLButtonState state),
                     void (*keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched,
                                                 uint32_t mods_locked, uint32_t group));

// Start the application.
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
void swcl_application_set_cursor(SWCLApplication *app, const char *name, size_t size);

// Shutdown SWCL application
void swcl_application_quit(SWCLApplication *app);

// ---------- WINDOW ---------- //

// Create new window.
// This function takes care of creating native wayland window with stuff like
// wl_surface, xdg_surface, xdg_toplevel and putting egl_window with OpenGL
// context into it.
SWCLWindow *swcl_window_new(SWCLApplication *app, char *title, size_t width, size_t height, size_t min_width,
                            size_t min_height, bool maximized, bool fullscreen, void (*draw_func)(SWCLWindow *win));

// Start window rendering.
void swcl_window_show(SWCLWindow *win);

// Show compositor window menu.
void swcl_window_show_menu(SWCLWindow *win);

// Tells compositor to begin native drag operation.
void swcl_window_drag(SWCLWindow *win);

// Tells compositor to begin native resize operation. With this, window can be
// resized if comositor allows it.
void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge);

// Swap OpenGL buffer for rendered frame
void swcl_window_swap_buffers(SWCLWindow *win);

// Set window title
void swcl_window_set_title(SWCLWindow *win, char *title);

// Set window size
void swcl_window_set_size(SWCLWindow *win, size_t width, size_t height);

// Set window minimum size
void swcl_window_set_min_size(SWCLWindow *win, size_t min_width, size_t min_height);

// Set window maximized state
void swcl_window_set_maximized(SWCLWindow *win, bool maximized);

// Set window minimized state
void swcl_window_minimize(SWCLWindow *win);

// Set window fullscreen state
void swcl_window_set_fullscreen(SWCLWindow *win, bool fullscreen);

#ifdef __cplusplus
}
#endif

#endif // SWCL_H

#ifdef SWCL_IMPLEMENTATION

#include <EGL/egl.h>
#include <GL/gl.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------- AUTOMATICALLY INSERTED PROTOCOLS FILES START ---------- //

#include "protocols/xdg-shell-protocol.h"

#include "protocols/xdg-shell-protocol.c"

// ---------- AUTOMATICALLY INSERTED PROTOCOLS FILES END ---------- //

// ---------- MACROS ---------- //

// Debug log formatted message
#ifdef SWCL_ENABLE_DEBUG_LOGS
#define SWCL_LOG_DEBUG(format, ...) fprintf(stdout, "SWCL DEBUG: " format "\n", ##__VA_ARGS__)
#else
#define SWCL_LOG_DEBUG(format, ...) ((void)0)
#endif // SWCL_ENABLE_DEBUG_LOGS

// Exit program with message
#define SWCL_PANIC(format, ...)                                                                                        \
  do {                                                                                                                 \
    fprintf(stderr, "SWCL PANIC: " format "\n", ##__VA_ARGS__);                                                        \
    exit(1);                                                                                                           \
  } while (0)

#define SWCL_ALLOC(T) (T *)malloc(sizeof(T))

// ---------- UTILS ---------- //

// Function to generate unique ID
static int swcl_generate_id() {
  static int id = -1; // static variable to hold the current ID value
  return ++id;        // increment and return the current ID
}

// Simple dynamic array structure that can only be grown in size
typedef struct {
  size_t length;
  size_t capacity;
  void **items;
} SWCLArray;

// Create new dynamic array with given initial capacity.
static inline SWCLArray swcl_array_new(size_t initial_capacity) {
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
    void **new_array = (void **)realloc(array->items, ++array->capacity * sizeof(void *));
    array->items = new_array;
  }
  array->items[array->length++] = item;
}

// Destroy array
static inline void swcl_array_free(SWCLArray array) {
  for (size_t i = 0; i < array.length; i++) free(array.items[i]);
}

// ------------ STRUCTS ------------ //

// Toplevel window object
struct SWCLWindow {
  // Read-Only properties
  size_t id;
  char *title;
  size_t width;
  size_t height;
  size_t min_width;
  size_t min_height;
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

// Position with x and y coordinates
typedef struct {
  size_t x;
  size_t y;
} SWCLPoint;

struct SWCLApplication {
  // Properties
  const char *app_id;
  bool running;
  SWCLArray windows;
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
  size_t cursor_pos_x;
  size_t cursor_pos_y;

  // Callbacks
  void (*pointer_enter_cb)(SWCLWindow *win, size_t x, size_t y);
  void (*pointer_motion_cb)(SWCLWindow *win, size_t x, size_t y);
  void (*pointer_leave_cb)(SWCLWindow *win);
  void (*mouse_button_cb)(SWCLWindow *win, SWCLMouseButton btn, SWCLButtonState state);
  void (*mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*keyboard_key_cb)(SWCLWindow *win, size_t key, SWCLButtonState state);
  void (*keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
                              uint32_t group);
};

// ---------- xdg_wm_base events callbacks ---------- //

static void on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial) {
  xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = on_xdg_wm_base_ping,
};

// ---------- wl_pointer events callbacks ---------- //

static void on_wl_pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time,
                                 uint32_t button, uint32_t state) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->wl_pointer_serial = serial;
  if (app->mouse_button_cb) app->mouse_button_cb(app->current_window, (SWCLMouseButton)button, (SWCLButtonState)state);
}

static void on_wl_pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->wl_pointer_serial = serial;
  app->cursor_pos_x = wl_fixed_to_int(x);
  app->cursor_pos_y = wl_fixed_to_int(y);
  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = (SWCLWindow *)app->windows.items[i];
    if (win->wl_surface == surface) {
      app->current_window = win;
      if (app->pointer_enter_cb) app->pointer_enter_cb(app->current_window, app->cursor_pos_x, app->cursor_pos_y);
    }
  }
};

static void on_wl_pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->pointer_leave_cb) app->pointer_leave_cb(app->current_window);
  app->wl_pointer_serial = serial;
};

static void on_wl_pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = (SWCLApplication *)data;
  app->cursor_pos_x = wl_fixed_to_int(x);
  app->cursor_pos_y = wl_fixed_to_int(y);
  if (app->pointer_motion_cb) app->pointer_motion_cb(app->current_window, app->cursor_pos_x, app->cursor_pos_y);
};

static void on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis,
                               wl_fixed_t value) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->mouse_scroll_cb) app->mouse_scroll_cb(app->current_window, (SWCLScrollDirection)(1 ? value > 0 : 0));
};

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = on_wl_pointer_enter,
    .leave = on_wl_pointer_leave,
    .motion = on_wl_pointer_motion,
    .button = on_wl_pointer_button,
    .axis = on_wl_pointer_axis,
};

// ---------- wl_keyboard events callbacks ---------- //

static void on_wl_kb_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {}

static void on_wl_kb_focus_enter_surface(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                                         struct wl_surface *surface, struct wl_array *key) {}

static void on_wl_kb_focus_leave_surface(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                                         struct wl_surface *surface) {}

static void on_wl_kb_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->keyboard_key_cb) app->keyboard_key_cb(app->current_window, key, (SWCLButtonState)state);
}

static void on_wl_kb_mod(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed,
                         uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (app->keyboard_mod_key_cb)
    app->keyboard_mod_key_cb(app->current_window, mods_depressed, mods_latched, mods_locked, group);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = on_wl_kb_keymap,
    .enter = on_wl_kb_focus_enter_surface,
    .leave = on_wl_kb_focus_leave_surface,
    .key = on_wl_kb_key,
    .modifiers = on_wl_kb_mod,
};

// ---------- wl_seat events callbacks ---------- //

static void on_wl_seat_capabilities(void *data, struct wl_seat *seat, uint32_t capability) {
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

static void on_wl_seat_name(void *data, struct wl_seat *seat, const char *name) {}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = on_wl_seat_capabilities,
    .name = on_wl_seat_name,
};

// ---------- wl_registry events callbacks ---------- //

static void on_wl_registry_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                                  uint32_t version) {
  SWCLApplication *app = (SWCLApplication *)data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    app->wl_compositor = (struct wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    app->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(registry, id, &xdg_wm_base_interface, 2);
    xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, NULL);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 2);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    app->wl_seat = (struct wl_seat *)wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    app->wl_cursor_shm = (struct wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, 1);
  }
}

static void on_wl_registry_global_remove(void *data, struct wl_registry *registry, uint32_t id) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = on_wl_registry_global,
    .global_remove = on_wl_registry_global_remove,
};

// ------------ APPLICATION METHODS ------------ //

SWCLApplication *
swcl_application_new(const char *app_id, void (*pointer_enter_cb)(SWCLWindow *win, size_t x, size_t y),
                     void (*pointer_motion_cb)(SWCLWindow *win, size_t x, size_t y),
                     void (*pointer_leave_cb)(SWCLWindow *win),
                     void (*mouse_button_cb)(SWCLWindow *win, SWCLMouseButton btn, SWCLButtonState state),
                     void (*mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir),
                     void (*keyboard_key_cb)(SWCLWindow *win, size_t key, SWCLButtonState state),
                     void (*keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched,
                                                 uint32_t mods_locked, uint32_t group)) {
  SWCLApplication *app = SWCL_ALLOC(SWCLApplication);

  app->app_id = app_id;
  app->running = false;

  // Set callbacks
  app->pointer_enter_cb = pointer_enter_cb;
  app->pointer_leave_cb = pointer_leave_cb;
  app->pointer_motion_cb = pointer_motion_cb;
  app->mouse_button_cb = mouse_button_cb;
  app->mouse_scroll_cb = mouse_scroll_cb;
  app->keyboard_key_cb = keyboard_key_cb;
  app->keyboard_mod_key_cb = keyboard_mod_key_cb;

  app->wl_display = wl_display_connect(NULL);
  if (!app->wl_display) SWCL_PANIC("Failed to connect to wl_display");
  else SWCL_LOG_DEBUG("Connected to wl_display");

  app->wl_registry = wl_display_get_registry(app->wl_display);
  if (!app->wl_registry) SWCL_PANIC("Failed to connect to wl_registry");
  else SWCL_LOG_DEBUG("Connected to wl_registry");

  wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
  wl_display_roundtrip(app->wl_display);

  // Get EGLDisplay
  app->egl_display = eglGetDisplay(app->wl_display);
  if (app->egl_display == EGL_NO_DISPLAY) SWCL_PANIC("Failed to get EGLDisplay");
  else SWCL_LOG_DEBUG("Got EGLDisplay");

  // Init EGL
  EGLint major, minor;
  if (!eglInitialize(app->egl_display, &major, &minor)) SWCL_PANIC("Failed to init EGL");
  else SWCL_LOG_DEBUG("Initialized EGL");

  // Bind OpenGL ES API to EGL
  if (!eglBindAPI(EGL_OPENGL_API)) SWCL_PANIC("Failed to bind OpenGL to EGL");
  else SWCL_LOG_DEBUG("Binded OpenGL to EGL");

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
  if (eglChooseConfig(app->egl_display, config_attrs, &app->egl_config, 1, &num) == EGL_FALSE || num == 0) {
    SWCL_PANIC("Failed to choose EGL config");
  } else SWCL_LOG_DEBUG("Chosen EGL config");

  // Create EGL context
  const EGLint context_attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  app->egl_context = eglCreateContext(app->egl_display, app->egl_config, EGL_NO_CONTEXT, context_attrs);
  if (!app->egl_context) {
    SWCL_PANIC("Failed to create EGL context");
  } else SWCL_LOG_DEBUG("Created EGL context");

  glEnable(GL_MULTISAMPLE);

  app->windows = swcl_array_new(2);

  return app;
}

void swcl_application_run(SWCLApplication *app) {
  app->running = true;
  while (app->running) { wl_display_dispatch(app->wl_display); }
  // Cleanup
  swcl_array_free(app->windows);
  wl_display_disconnect(app->wl_display);
  free(app);
}

void swcl_application_quit(SWCLApplication *app) { app->running = false; }

void swcl_application_set_cursor(SWCLApplication *app, const char *name, size_t size) {
  if (!app->current_cursor_name) goto create;
  if (!strcmp(app->current_cursor_name, name)) goto update;

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
  wl_surface_damage(app->wl_cursor_surface, 0, 0, app->wl_cursor_image->width, app->wl_cursor_image->height);
  wl_surface_commit(app->wl_cursor_surface);
  wl_pointer_set_cursor(app->wl_pointer, app->wl_pointer_serial, app->wl_cursor_surface,
                        app->wl_cursor_image->hotspot_x, app->wl_cursor_image->hotspot_y);
  app->current_cursor_name = (char *)name;
}

// ---------- xdg_toplevel events callbacks ---------- //

static void on_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height,
                                      struct wl_array *states) {
  if (width == 0 || height == 0) return;
  SWCL_LOG_DEBUG("xdg_toplevel configure. width=%d, height=%d", width, height);
  SWCLWindow *win = (SWCLWindow *)data;

  // Loop over the states array
  uint32_t *state;
  for (state = (uint32_t *)(states)->data; (const char *)state < ((const char *)(states)->data + (states)->size);
       (state)++) {
    switch (*state) {
    // Set maximized state
    case XDG_TOPLEVEL_STATE_MAXIMIZED: win->maximized = true;
    // Set unmaximized state
    case XDG_TOPLEVEL_STATE_ACTIVATED:
      if (win->maximized && (win->width > width || win->height > height)) { win->maximized = false; }
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

static void on_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  SWCLWindow *win = (SWCLWindow *)data;
  win->app->running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = on_xdg_toplevel_configure,
    .close = on_xdg_toplevel_close,
};

// ---------- xdg_surface events callbacks ---------- //

static void on_xdg_surface_configure(void *data, struct xdg_surface *surface, uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = on_xdg_surface_configure,
};

// ---------- wl_callback events callback ---------- //

static void on_new_frame(void *data, struct wl_callback *cb, uint32_t cb_data);
static void swcl__window_make_current(SWCLWindow *win);

static const struct wl_callback_listener wl_callback_listener = {
    .done = on_new_frame,
};

static void on_new_frame(void *data, struct wl_callback *cb, uint32_t cb_data) {
  SWCLWindow *win = (SWCLWindow *)data;
  swcl__window_make_current(win);
  wl_callback_destroy(cb);
  cb = wl_surface_frame(win->wl_surface);
  wl_callback_add_listener(cb, &wl_callback_listener, win);
  win->on_draw_cb(win);
}

// ------------ WINDOW ------------ //

static void swcl__window_make_current(SWCLWindow *win) {
  if (eglMakeCurrent(win->app->egl_display, win->egl_surface, win->egl_surface, win->app->egl_context) == EGL_FALSE)
    SWCL_PANIC("Failed to make egl_surface current");
}

SWCLWindow *swcl_window_new(SWCLApplication *app, char *title, size_t width, size_t height, size_t min_width,
                            size_t min_height, bool maximized, bool fullscreen, void (*draw_func)(SWCLWindow *win)) {

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

  SWCL_LOG_DEBUG("Create new window with id: %d, width: %d, height: %d", win->id, win->width, win->height);

  // Get wl_surface
  if (!app->wl_compositor) SWCL_PANIC("Failed to get wl_compositor");

  win->wl_surface = wl_compositor_create_surface(app->wl_compositor);
  if (!win->wl_surface) SWCL_PANIC("Failed to get wl_surface");
  else SWCL_LOG_DEBUG("Got wl_surface");

  // Get wl_callback for surface frame
  win->wl_callback = wl_surface_frame(win->wl_surface);
  if (!win->wl_callback) SWCL_PANIC("Failed to get wl_callback");
  else {
    SWCL_LOG_DEBUG("Got wl_callback");
    wl_callback_add_listener(win->wl_callback, &wl_callback_listener, win);
    wl_display_roundtrip(app->wl_display);
  }

  // Get xdg_surface
  win->xdg_surface = xdg_wm_base_get_xdg_surface(app->xdg_wm_base, win->wl_surface);
  if (!win->xdg_surface) SWCL_PANIC("Failed to get xdg_surface");
  else {
    SWCL_LOG_DEBUG("Got xdg_surface");
    xdg_surface_add_listener(win->xdg_surface, &xdg_surface_listener, NULL);
  }

  // Get xdg_toplevel
  win->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
  if (!win->xdg_toplevel) SWCL_PANIC("Failed to get xdg_toplevel");
  else {
    SWCL_LOG_DEBUG("Got xdg_toplevel");
    xdg_toplevel_add_listener(win->xdg_toplevel, &xdg_toplevel_listener, win);
    wl_display_roundtrip(app->wl_display);
  }

  // Create EGL window
  win->egl_window = wl_egl_window_create(win->wl_surface, win->width, win->height);

  // Create EGL surface
  win->egl_surface =
      eglCreateWindowSurface(app->egl_display, app->egl_config, (EGLNativeWindowType)win->egl_window, NULL);
  if (win->egl_surface == EGL_NO_SURFACE) SWCL_PANIC("Failed to create EGL surface");
  else SWCL_LOG_DEBUG("Created EGL surface");

  // Setup xdg_toplevel
  xdg_toplevel_set_app_id(win->xdg_toplevel, app->app_id);
  // Set title
  if (win->title) xdg_toplevel_set_title(win->xdg_toplevel, win->title);
  // Set min size
  if (win->min_height && win->min_width) xdg_toplevel_set_min_size(win->xdg_toplevel, win->min_width, win->min_height);
  // Set maximized
  if (win->maximized) xdg_toplevel_set_maximized(win->xdg_toplevel);
  else xdg_toplevel_unset_maximized(win->xdg_toplevel);
  // Set fullscreen
  if (win->fullscreen) xdg_toplevel_set_fullscreen(win->xdg_toplevel, NULL);
  else xdg_toplevel_unset_fullscreen(win->xdg_toplevel);

  swcl_array_append(&app->windows, win);
  SWCL_LOG_DEBUG("Created window with id=%d, at %p", win->id, win);
  return win;
}

void swcl_window_show(SWCLWindow *win) {
  swcl__window_make_current(win);
  win->on_draw_cb(win);
}

void swcl_window_drag(SWCLWindow *win) {
  xdg_toplevel_move(win->xdg_toplevel, win->app->wl_seat, win->app->wl_pointer_serial);
}

void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge) {
  xdg_toplevel_resize(win->xdg_toplevel, win->app->wl_seat, win->app->wl_pointer_serial, edge);
}

void swcl_window_swap_buffers(SWCLWindow *win) { eglSwapBuffers(win->app->egl_display, win->egl_surface); }

void swcl_window_set_title(SWCLWindow *win, char *title) {
  win->title = title;
  xdg_toplevel_set_title(win->xdg_toplevel, title);
}

void swcl_window_set_maximized(SWCLWindow *win, bool maximized) {
  win->maximized = maximized;
  if (maximized) xdg_toplevel_set_maximized(win->xdg_toplevel);
  else xdg_toplevel_unset_maximized(win->xdg_toplevel);
}

void swcl_window_minimize(SWCLWindow *win) { xdg_toplevel_set_minimized(win->xdg_toplevel); }

void swcl_window_set_min_size(SWCLWindow *win, size_t min_width, size_t min_height) {
  win->min_width = min_width;
  win->min_height = min_height;
  xdg_toplevel_set_min_size(win->xdg_toplevel, min_width, min_height);
}

#endif // SWCL_IMPLEMENTATION
