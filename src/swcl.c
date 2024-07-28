#include "swcl.h"
#include "xdg-shell-protocol.h"

#include <GLES2/gl2.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>

#include <string.h>

// --------------------------------- //
//              GLOBALS              //
// --------------------------------- //

// Wayland
struct wl_display *swcl_wl_display;
struct wl_registry *swcl_wl_registry;
struct wl_compositor *swcl_wl_compositor;
struct wl_seat *swcl_wl_seat;
struct wl_pointer *swcl_wl_pointer;
struct wl_keyboard *swcl_wl_keyboard;
struct xdg_wm_base *swcl_xdg_wm_base;

uint32_t wl_pointer_last_serial;

// EGL
EGLConfig swcl_egl_config;
EGLDisplay swcl_egl_display;
EGLContext swcl_egl_context;

const char *swcl_app_id;
bool swcl_app_running;
SWCLPoint swcl_cursor_pos;
int swcl_current_window_id;

SWCLArray swcl_windows;
SWCLWindow *current_window;

// Cursor
struct wl_buffer *swcl_wl_cursor_buffer;
struct wl_cursor *swcl_wl_cursor;
struct wl_cursor_theme *swcl_wl_cursor_theme;
struct wl_cursor_image *swcl_wl_cursor_image;
struct wl_shm *swcl_wl_cursor_shm;
struct wl_surface *swcl_wl_cursor_surface;

// --------------------------------- //
//              DRAWING              //
// --------------------------------- //

void swcl_clear_background(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT);
}

// --------------------------------- //
//              CURSOR               //
// --------------------------------- //

SWCLCursor swcl_cursor_new(const char *name) {
  SWCLCursor cur;
  cur.wl_cursor_theme = wl_cursor_theme_load(NULL, 24, cur.wl_shm);
  cur.wl_cursor = wl_cursor_theme_get_cursor(cur.wl_cursor_theme, name);
  cur.wl_cursor_image = cur.wl_cursor->images[0];
  cur.wl_buffer = wl_cursor_image_get_buffer(cur.wl_cursor_image);
  cur.wl_surface = wl_compositor_create_surface(swcl_wl_compositor);
  wl_surface_attach(cur.wl_surface, cur.wl_buffer, 0, 0);
  wl_surface_commit(cur.wl_surface);
  return cur;
}

// --------------------------------- //
//               WINDOW              //
// --------------------------------- //

void swcl_window_make_current(SWCLWindow *win);

// -------- xdg_toplevel events callbacks -------- //

static void on_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array *states) {
  if (width == 0 || height == 0)
    return;
  SWCL_LOG_DEBUG("xdg_toplevel configure. width=%d, height=%d", width, height);
  SWCLWindow *win = data;

  // Pointer to iterate over the states
  uint32_t *state;

  // Loop over the states array
  wl_array_for_each(state, states) {
    switch (*state) {
    case XDG_TOPLEVEL_STATE_MAXIMIZED:
      SWCL_LOG_DEBUG("State: Maximized");
      break;
    }
  }

  // Resize window if needed
  if (win->egl_window) {
    win->width = (int)width;
    win->height = (int)height;
    glViewport(0, 0, width, height);
    wl_egl_window_resize(win->egl_window, width, height, 0, 0);
  }
}

static void on_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  SWCLWindow *win = data;
  swcl_app_running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = on_xdg_toplevel_configure,
    .close = on_xdg_toplevel_close,
};

// -------- xdg_surface events callbacks -------- //

static void on_xdg_surface_configure(void *data, struct xdg_surface *surface,
                                     uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);
}

static struct xdg_surface_listener xdg_surface_listener = {
    .configure = on_xdg_surface_configure,
};

// -------- wl_callback events callback -------- //

static void on_new_frame(void *data, struct wl_callback *cb, uint32_t cb_data);
static struct wl_callback_listener wl_callback_listener = {
    .done = on_new_frame,
};

static void on_new_frame(void *data, struct wl_callback *cb, uint32_t cb_data) {
  SWCLWindow *win = data;
  swcl_window_make_current(win);
  wl_callback_destroy(cb);
  cb = wl_surface_frame(win->wl_surface);
  wl_callback_add_listener(cb, &wl_callback_listener, win);
  win->on_draw_cb(win);
}

// ---------- WINDOW ---------- //

SWCLWindow *swcl_window_new(SWCLWindowConfig cfg) {
  SWCLWindow *win = malloc(sizeof(*win));

  win->id = swcl_generate_id();
  win->width = cfg.width;
  win->height = cfg.height;
  win->min_width = cfg.min_width;
  win->min_height = cfg.min_height;
  win->maximized = cfg.maximized;
  win->fullscreen = cfg.fullscreen;
  win->title = (char *)malloc(strlen(cfg.title) + 1);
  strcpy(win->title, cfg.title);

  win->on_draw_cb = cfg.on_draw_cb;

  SWCL_LOG_DEBUG("Create new window with id: %d, width: %d, height: %d",
                 win->id, win->width, win->height);

  // ---------- WAYLAND STUFF ---------- //

  // Get wl_surface
  win->wl_surface = wl_compositor_create_surface(swcl_wl_compositor);
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
    wl_display_roundtrip(swcl_wl_display);
  }

  // Get xdg_surface
  win->xdg_surface =
      xdg_wm_base_get_xdg_surface(swcl_xdg_wm_base, win->wl_surface);
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
    wl_display_roundtrip(swcl_wl_display);
  }

  // ---------- EGL STUFF ---------- //

  // Create EGL window
  win->egl_window =
      wl_egl_window_create(win->wl_surface, win->width, win->height);

  // Create EGL surface
  win->egl_surface =
      eglCreateWindowSurface(swcl_egl_display, swcl_egl_config,
                             (EGLNativeWindowType)win->egl_window, NULL);
  if (win->egl_surface == EGL_NO_SURFACE)
    SWCL_PANIC("Failed to create EGL surface");
  else
    SWCL_LOG_DEBUG("Created EGL surface");

  // Enable multisampling for smoothing
  // glEnable(GL_MULTISAMPLE);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  swcl_window_make_current(win);
  win->on_draw_cb(win);

  // Setup xdg_toplevel
  xdg_toplevel_set_app_id(win->xdg_toplevel, swcl_app_id);
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

  swcl_array_append(&swcl_windows, win);
  SWCL_LOG_DEBUG("Created window with id=%d, at %p", win->id, win);
  return win;
}

void swcl_window_make_current(SWCLWindow *win) {
  if (eglMakeCurrent(swcl_egl_display, win->egl_surface, win->egl_surface,
                     swcl_egl_context) == EGL_FALSE)
    SWCL_PANIC("Failed to make egl_surface current");
}

void swcl_window_drag(SWCLWindow *win, uint32_t serial) {
  xdg_toplevel_move(win->xdg_toplevel, swcl_wl_seat, serial);
}

void swcl_window_resize(SWCLWindow *win, uint32_t serial, SWCLWindowEdge edge) {
  xdg_toplevel_resize(win->xdg_toplevel, swcl_wl_seat, serial, edge);
}

void swcl_window_swap_buffers(SWCLWindow *win) {
  eglSwapBuffers(swcl_egl_display, win->egl_surface);
}

int swcl_window_get_id(SWCLWindow *win) { return win->id; }
char *swcl_window_get_title(SWCLWindow *win) { return win->title; }
int swcl_window_get_width(SWCLWindow *win) { return win->width; }
int swcl_window_get_height(SWCLWindow *win) { return win->height; }
int swcl_window_get_min_width(SWCLWindow *win) { return win->min_width; }
int swcl_window_get_min_height(SWCLWindow *win) { return win->min_height; }
bool swcl_window_get_maximized(SWCLWindow *win) { return win->maximized; }

void swcl_window_set_title(SWCLWindow *win, char *title) {
  if (win->title)
    free(win->title);
  win->title = (char *)realloc(win->title, strlen(title) + 1);
  strcpy(win->title, title);
  xdg_toplevel_set_title(win->xdg_toplevel, title);
}

void swcl_window_set_maximized(SWCLWindow *win, bool maximized) {
  win->maximized = maximized;
  if (maximized)
    xdg_toplevel_set_maximized(win->xdg_toplevel);
  else
    xdg_toplevel_unset_maximized(win->xdg_toplevel);
}

void swcl_window_set_min_size(SWCLWindow *win, int min_width, int min_height) {
  if (min_width <= 0 || min_height <= 0)
    return;
  win->min_width = min_width;
  win->min_height = min_height;
  xdg_toplevel_set_min_size(win->xdg_toplevel, min_width, min_height);
}

// void swcl_window_set_cursor(SWCLWindow *win, const char *name,
//                             uint32_t serial) {
//   SWCLCursor cur = swcl_cursor_new(win, name);
//   wl_pointer_set_cursor(swcl_wl_pointer, serial, cur.wl_surface,
//                         cur.wl_cursor_image->hotspot_x,
//                         cur.wl_cursor_image->hotspot_y);
// }

// --------------------------------- //
//            APPLICATION            //
// --------------------------------- //

// -------- xdg_wm_base events callbacks -------- //

static void on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base,
                                uint32_t serial) {
  xdg_wm_base_pong(wm_base, serial);
}

static void on_wl_seat_name(void *data, struct wl_seat *seat,
                            const char *name) {}

static struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = on_xdg_wm_base_ping,
};

// -------- wl_pointer events callbacks -------- //

static void on_wl_pointer_button(void *data, struct wl_pointer *pointer,
                                 uint32_t serial, uint32_t time,
                                 uint32_t button, uint32_t state) {
  SWCLConfig *cfg = data;
  if (cfg->on_mouse_button_cb)
    cfg->on_mouse_button_cb(current_window, button, state, serial);
  wl_pointer_last_serial = serial;
}

static void on_wl_pointer_enter(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t x, wl_fixed_t y) {
  SWCLConfig *cfg = data;

  swcl_cursor_pos.x = wl_fixed_to_int(x);
  swcl_cursor_pos.y = wl_fixed_to_int(y);

  for (int i = 0; i < swcl_windows.length; i++) {
    SWCLWindow *win = swcl_windows.items[i];
    if (win->wl_surface == surface) {
      current_window = win;
      swcl_current_window_id = win->id;
      if (cfg->on_pointer_enter_cb)
        cfg->on_pointer_enter_cb(current_window, swcl_cursor_pos.x,
                                 swcl_cursor_pos.y, serial);
    }
  }
  wl_pointer_last_serial = serial;
};

static void on_wl_pointer_leave(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface) {
  SWCLConfig *cfg = data;
  if (cfg->on_pointer_leave_cb)
    cfg->on_pointer_leave_cb(current_window, serial);
  wl_pointer_last_serial = serial;
};

static void on_wl_pointer_motion(void *data, struct wl_pointer *pointer,
                                 uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  SWCLConfig *cfg = data;
  swcl_cursor_pos.x = wl_fixed_to_int(x);
  swcl_cursor_pos.y = wl_fixed_to_int(y);
  if (cfg->on_pointer_motion_cb)
    cfg->on_pointer_motion_cb(current_window, swcl_cursor_pos.x,
                              swcl_cursor_pos.y);
};

static void on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                               uint32_t time, uint32_t axis, wl_fixed_t value) {
  SWCLConfig *cfg = data;
  if (cfg->on_mouse_scroll_cb)
    cfg->on_mouse_scroll_cb(current_window, 1 ? value > 0 : 0);
};

static struct wl_pointer_listener wl_pointer_listener = {
    .enter = on_wl_pointer_enter,
    .leave = on_wl_pointer_leave,
    .motion = on_wl_pointer_motion,
    .button = on_wl_pointer_button,
    .axis = on_wl_pointer_axis,
};

// -------- wl_keyboard events callbacks -------- //

void on_wl_kb_keymap(void *data, struct wl_keyboard *wl_keyboard,
                     uint32_t format, int32_t fd, uint32_t size) {}

void on_wl_kb_focus_enter_surface(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, struct wl_surface *surface,
                                  struct wl_array *key) {}

void on_wl_kb_focus_leave_surface(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, struct wl_surface *surface) {
}

void on_wl_kb_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  uint32_t time, uint32_t key, uint32_t state) {
  SWCLConfig *cfg = data;
  if (cfg->on_keyboard_key_cb)
    cfg->on_keyboard_key_cb(current_window, key, state, serial);
}

void on_wl_kb_mod(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  uint32_t mods_depressed, uint32_t mods_latched,
                  uint32_t mods_locked, uint32_t group) {
  SWCLConfig *cfg = data;
  if (cfg->on_keyboard_mod_key_cb)
    cfg->on_keyboard_mod_key_cb(current_window, mods_depressed, mods_latched,
                                mods_locked, group, serial);
}

static struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = on_wl_kb_keymap,
    .enter = on_wl_kb_focus_enter_surface,
    .leave = on_wl_kb_focus_leave_surface,
    .key = on_wl_kb_key,
    .modifiers = on_wl_kb_mod,
};

// -------- wl_seat events callbacks -------- //

static void on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                    uint32_t capability) {
  SWCLConfig *cfg = data;
  if (!cfg) {
    SWCL_LOG_DEBUG("SWCLConfig is NULL");
    return;
  }

  if ((capability & WL_SEAT_CAPABILITY_POINTER) && !swcl_wl_pointer) {
    swcl_wl_pointer = wl_seat_get_pointer(seat);
    if (swcl_wl_pointer) {
      SWCL_LOG_DEBUG("Got pointer");
      wl_pointer_add_listener(swcl_wl_pointer, &wl_pointer_listener, cfg);
      wl_display_roundtrip(swcl_wl_display);
    } else {
      SWCL_LOG_DEBUG("No pointer found");
    }
  }

  if ((capability & WL_SEAT_CAPABILITY_KEYBOARD) && !swcl_wl_keyboard) {
    swcl_wl_keyboard = wl_seat_get_keyboard(seat);
    if (swcl_wl_keyboard) {
      SWCL_LOG_DEBUG("Got keyboard");
      wl_keyboard_add_listener(swcl_wl_keyboard, &wl_keyboard_listener, cfg);
      wl_display_roundtrip(swcl_wl_display);
    } else {
      SWCL_LOG_DEBUG("No keyboard found");
    }
  }
}

static struct wl_seat_listener wl_seat_listener = {
    .capabilities = on_wl_seat_capabilities,
    .name = on_wl_seat_name,
};

// -------- wl_registry events callbacks -------- //

static void on_wl_registry_global(void *data, struct wl_registry *registry,
                                  uint32_t id, const char *interface,
                                  uint32_t version) {
  SWCLConfig *cfg = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    swcl_wl_compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    swcl_xdg_wm_base =
        wl_registry_bind(registry, id, &xdg_wm_base_interface, 2);
    xdg_wm_base_add_listener(swcl_xdg_wm_base, &xdg_wm_base_listener, NULL);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 2);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    swcl_wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(swcl_wl_seat, &wl_seat_listener, cfg);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    swcl_wl_cursor_shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
  }
}

static void on_wl_registry_global_remove(void *data,
                                         struct wl_registry *registry,
                                         uint32_t id) {}

static struct wl_registry_listener wl_registry_listener = {
    .global = on_wl_registry_global,
    .global_remove = on_wl_registry_global_remove,
};

// ---------- MAIN FUNCTIONS ---------- //

bool swcl_init(SWCLConfig *cfg) {
  SWCL_LOG("Create application with id: %s", cfg->app_id);

  swcl_app_id = cfg->app_id;

  // Connect to display
  swcl_wl_display = wl_display_connect(NULL);
  if (!swcl_wl_display) {
    SWCL_LOG("Failed to connect to wl_display");
    return false;
  } else
    SWCL_LOG_DEBUG("Connected to wl_display");

  // Connect to registry
  swcl_wl_registry = wl_display_get_registry(swcl_wl_display);
  if (!swcl_wl_registry) {
    SWCL_LOG("Failed to connect to wl_registry");
    return false;
  } else
    SWCL_LOG_DEBUG("Connected to wl_registry");

  wl_registry_add_listener(swcl_wl_registry, &wl_registry_listener, cfg);
  wl_display_roundtrip(swcl_wl_display);

  // Get EGLDisplay
  swcl_egl_display = eglGetDisplay(swcl_wl_display);
  if (swcl_egl_display == EGL_NO_DISPLAY) {
    SWCL_LOG("Failed to get EGLDisplay");
    return false;
  } else
    SWCL_LOG_DEBUG("Got EGLDisplay");

  // Init EGL
  EGLint major, minor;
  if (eglInitialize(swcl_egl_display, &major, &minor) == EGL_FALSE) {
    SWCL_LOG("Failed to init EGL");
    return false;
  } else
    SWCL_LOG_DEBUG("Initialized EGL");

  // Bind OpenGL ES API to EGL
  if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
    SWCL_LOG("Failed to bind OpenGL ES to EGL");
    return false;
  } else
    SWCL_LOG_DEBUG("Binded OpenGL to EGL");

  EGLint config_attrs[] = {EGL_SURFACE_TYPE,
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
                           EGL_OPENGL_ES2_BIT,
                           EGL_SAMPLE_BUFFERS,
                           1, // Enable multi-sampling
                           EGL_SAMPLES,
                           4, // Number of samples
                           EGL_NONE};

  // Choose config
  EGLint num;
  if (eglChooseConfig(swcl_egl_display, config_attrs, &swcl_egl_config, 1,
                      &num) == EGL_FALSE ||
      num == 0) {
    SWCL_LOG("Failed to choose EGL config");
    return false;
  } else
    SWCL_LOG_DEBUG("Chosen EGL config");

  // Create EGL context
  EGLint context_attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  swcl_egl_context = eglCreateContext(swcl_egl_display, swcl_egl_config,
                                      EGL_NO_CONTEXT, context_attrs);
  if (swcl_egl_context == EGL_NO_CONTEXT) {
    SWCL_LOG("Failed to create EGL context");
    return false;
  } else
    SWCL_LOG_DEBUG("Created EGL context");

  // Create windows array
  swcl_windows = swcl_array_new(2);

  return true;
}

void swcl_run() {
  swcl_app_running = true;
  while (swcl_app_running) {
    wl_display_dispatch(swcl_wl_display);
  }
}

void swcl_quit() { swcl_app_running = false; }

void swcl_set_cursor(const char *name, uint8_t size, uint32_t serial) {
  if (!swcl_wl_cursor_surface)
    swcl_wl_cursor_surface = wl_compositor_create_surface(swcl_wl_compositor);
  swcl_wl_cursor_theme = wl_cursor_theme_load(NULL, size, swcl_wl_cursor_shm);
  swcl_wl_cursor = wl_cursor_theme_get_cursor(swcl_wl_cursor_theme, name);
  swcl_wl_cursor_image = swcl_wl_cursor->images[0];
  swcl_wl_cursor_buffer = wl_cursor_image_get_buffer(swcl_wl_cursor_image);
  wl_pointer_set_cursor(swcl_wl_pointer,
                        serial ? serial != 0 : wl_pointer_last_serial,
                        swcl_wl_cursor_surface, swcl_wl_cursor_image->hotspot_x,
                        swcl_wl_cursor_image->hotspot_y);
  wl_surface_attach(swcl_wl_cursor_surface, swcl_wl_cursor_buffer, 0, 0);
  wl_surface_damage(swcl_wl_cursor_surface, 0, 0, swcl_wl_cursor_image->width,
                    swcl_wl_cursor_image->height);
  wl_surface_commit(swcl_wl_cursor_surface);
}
