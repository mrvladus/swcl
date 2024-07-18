#include "swcl.h"
#include "window.h"

#include "xdg-shell-client-protocol.h"

#include <EGL/egl.h>
#include <wayland-client.h>

#include <string.h>

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
  SWCLApplication *app = data;
  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->id == app->current_window_id && win->on_mouse_button_cb) {
      win->on_mouse_button_cb(win, button, state, serial);
      return;
    }
  }
}

static void on_wl_pointer_enter(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = data;
  app->cur_pos.x = wl_fixed_to_int(x);
  app->cur_pos.y = wl_fixed_to_int(y);

  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->wl_surface == surface) {
      if (win->on_pointer_enter_cb) {
        win->on_pointer_enter_cb(win, wl_fixed_to_int(x), wl_fixed_to_int(y));
        app->current_window_id = win->id;
        return;
      }
    }
  }
};

static void on_wl_pointer_leave(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface) {
  SWCLApplication *app = data;

  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->wl_surface == surface && win->on_pointer_leave_cb) {
      win->on_pointer_leave_cb(win);
      return;
    }
  }
};

static void on_wl_pointer_motion(void *data, struct wl_pointer *pointer,
                                 uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = data;
  app->cur_pos.x = wl_fixed_to_int(x);
  app->cur_pos.y = wl_fixed_to_int(y);

  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->id == app->current_window_id)
      if (win->on_pointer_motion_cb) {
        win->on_pointer_motion_cb(win, app->cur_pos.x, app->cur_pos.y);
        return;
      }
  }
};

static void on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                               uint32_t time, uint32_t axis, wl_fixed_t value) {
  SWCLApplication *app = data;

  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->id == app->current_window_id && win->on_mouse_scroll_cb) {
      win->on_mouse_scroll_cb(win, 1 ? value > 0 : 0);
      return;
    }
  }
};

static struct wl_pointer_listener wl_pointer_listener = {
    .enter = on_wl_pointer_enter,
    .leave = on_wl_pointer_leave,
    .motion = on_wl_pointer_motion,
    .button = on_wl_pointer_button,
    .axis = on_wl_pointer_axis,
};

// -------- wl_seat events callbacks -------- //

static void on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                    uint32_t capability) {

  SWCLApplication *app = data;

  // Get pointer
  app->wl_pointer = wl_seat_get_pointer(app->wl_seat);

  wl_pointer_add_listener(app->wl_pointer, &wl_pointer_listener, app);
  if (!app->wl_pointer)
    SWCL_LOG_DEBUG("No pointer found");
  else
    SWCL_LOG_DEBUG("Got pointer");

  // Get keyboard
  app->wl_keyboard = wl_seat_get_keyboard(seat);
  if (!app->wl_keyboard)
    SWCL_LOG_DEBUG("No pointer found");
  else
    SWCL_LOG_DEBUG("Got keyboard");
}

static struct wl_seat_listener wl_seat_listener = {
    .capabilities = on_wl_seat_capabilities,
    .name = on_wl_seat_name,
};

// -------- wl_registry events callbacks -------- //

static void on_wl_registry_global(void *data, struct wl_registry *registry,
                                  uint32_t id, const char *interface,
                                  uint32_t version) {
  SWCLApplication *app = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    app->wl_compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    app->xdg_wm_base =
        wl_registry_bind(registry, id, &xdg_wm_base_interface, 2);
    xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, NULL);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 2);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    app->wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  }
}

static void on_wl_registry_global_remove(void *data,
                                         struct wl_registry *registry,
                                         uint32_t id) {}

static struct wl_registry_listener wl_registry_listener = {
    .global = on_wl_registry_global,
    .global_remove = on_wl_registry_global_remove,
};

SWCLApplication *swcl_application_new(char *app_id) {
  SWCL_LOG("Create application with id: %s", app_id);

  SWCLApplication *app = malloc(sizeof(*app));

  // Set app id
  app->app_id = app_id;

  // ---------- WAYLAND STUFF ---------- //

  // Connect to display
  app->wl_display = wl_display_connect(NULL);
  if (!app->wl_display)
    SWCL_PANIC("Failed to connect to wl_display");
  else
    SWCL_LOG_DEBUG("Connected to wl_display");

  // Connect to registry
  app->wl_registry = wl_display_get_registry(app->wl_display);
  if (!app->wl_registry)
    SWCL_PANIC("Failed to connect to wl_registry");
  else
    SWCL_LOG_DEBUG("Connected to wl_registry");

  wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
  wl_display_roundtrip(app->wl_display);

  // ---------- EGL STUFF ---------- //

  // Get EGLDisplay
  app->egl_display = eglGetDisplay(app->wl_display);
  if (app->egl_display == EGL_NO_DISPLAY)
    SWCL_PANIC("Failed to get EGLDisplay");
  else
    SWCL_LOG_DEBUG("Got EGLDisplay");

  // Init EGL
  EGLint major, minor;
  if (eglInitialize(app->egl_display, &major, &minor) == EGL_FALSE)
    SWCL_PANIC("Failed to init EGL");
  else
    SWCL_LOG_DEBUG("Initialized EGL");

  // Bind OpenGL ES API to EGL
  if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    SWCL_PANIC("Failed to bind OpenGL ES to EGL");
  else
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
                           EGL_OPENGL_ES3_BIT,
                           EGL_SAMPLE_BUFFERS,
                           1, // Enable multi-sampling
                           EGL_SAMPLES,
                           4, // Number of samples
                           EGL_NONE};

  // Choose config
  EGLint num;
  if (eglChooseConfig(app->egl_display, config_attrs, &app->egl_config, 1,
                      &num) == EGL_FALSE ||
      num == 0)
    SWCL_PANIC("Failed to choose EGL config");
  else
    SWCL_LOG_DEBUG("Chosen EGL config");

  // Create EGL context
  EGLint context_attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  app->egl_context = eglCreateContext(app->egl_display, app->egl_config,
                                      EGL_NO_CONTEXT, context_attrs);
  if (app->egl_context == EGL_NO_CONTEXT)
    SWCL_PANIC("Failed to create EGL context");
  else
    SWCL_LOG_DEBUG("Created EGL context");

  // Create windows array
  app->windows = swcl_array_new(2);

  return app;
}

void swcl_application_run(SWCLApplication *app) {
  app->running = true;
  while (app->running) {
    wl_display_dispatch(app->wl_display);
  }
}

SWCLPoint swcl_application_get_cursor_position(SWCLApplication *app) {
  return app->cur_pos;
}
