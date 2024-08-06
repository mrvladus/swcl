#include "swcl.h"

#include "wlr-layer-shell-protocol.h"
#include "xdg-shell-protocol.h"

#include <EGL/egl.h>
#include <GL/gl.h>
#include <wayland-client-core.h>
#include <wayland-cursor.h>

#include <string.h>

// -------- xdg_wm_base events callbacks -------- //

static void on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base,
                                uint32_t serial) {
  xdg_wm_base_pong(wm_base, serial);
}

static void on_wl_seat_name(void *data, struct wl_seat *seat,
                            const char *name) {}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = on_xdg_wm_base_ping,
};

// -------- wl_pointer events callbacks -------- //

static void on_wl_pointer_button(void *data, struct wl_pointer *pointer,
                                 uint32_t serial, uint32_t time,
                                 uint32_t button, uint32_t state) {
  SWCLApplication *app = data;
  app->wl_pointer_serial = serial;
  if (app->on_mouse_button_cb)
    app->on_mouse_button_cb(app->current_window, button, state);
}

static void on_wl_pointer_enter(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = data;
  app->wl_pointer_serial = serial;
  app->cursor_pos.x = wl_fixed_to_int(x);
  app->cursor_pos.y = wl_fixed_to_int(y);
  for (int i = 0; i < app->windows.length; i++) {
    SWCLWindow *win = app->windows.items[i];
    if (win->wl_surface == surface) {
      app->current_window = win;
      if (app->on_pointer_enter_cb)
        app->on_pointer_enter_cb(app->current_window, app->cursor_pos.x,
                                 app->cursor_pos.y);
    }
  }
};

static void on_wl_pointer_leave(void *data, struct wl_pointer *pointer,
                                uint32_t serial, struct wl_surface *surface) {
  SWCLApplication *app = data;
  if (app->on_pointer_leave_cb)
    app->on_pointer_leave_cb(app->current_window);
  app->wl_pointer_serial = serial;
};

static void on_wl_pointer_motion(void *data, struct wl_pointer *pointer,
                                 uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  SWCLApplication *app = data;
  app->cursor_pos.x = wl_fixed_to_int(x);
  app->cursor_pos.y = wl_fixed_to_int(y);
  if (app->on_pointer_motion_cb)
    app->on_pointer_motion_cb(app->current_window, app->cursor_pos.x,
                              app->cursor_pos.y);
};

static void on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                               uint32_t time, uint32_t axis, wl_fixed_t value) {
  SWCLApplication *app = data;
  if (app->on_mouse_scroll_cb)
    app->on_mouse_scroll_cb(app->current_window, 1 ? value > 0 : 0);
};

static const struct wl_pointer_listener wl_pointer_listener = {
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
  SWCLApplication *app = data;
  if (app->on_keyboard_key_cb)
    app->on_keyboard_key_cb(app->current_window, key, state);
}

void on_wl_kb_mod(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  uint32_t mods_depressed, uint32_t mods_latched,
                  uint32_t mods_locked, uint32_t group) {
  SWCLApplication *app = data;
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

static void on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                    uint32_t capability) {
  SWCLApplication *app = data;

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
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    app->wl_cursor_shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    app->wlr_layer_shell =
        wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
    SWCL_LOG_DEBUG("Registered %s version %d", interface, 1);
  }
}

static void on_wl_registry_global_remove(void *data,
                                         struct wl_registry *registry,
                                         uint32_t id) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = on_wl_registry_global,
    .global_remove = on_wl_registry_global_remove,
};

// ---------- APPLICATION ---------- //

SWCLApplication *swcl_application_new(SWCLConfig *cfg) {
  SWCLApplication *app = malloc(sizeof(*app));

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
  // TODO: Make OpenGL API configurable
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_MULTISAMPLE);

  app->windows = swcl_array_new(2);

  return app;
}

void swcl_application_run(SWCLApplication *app) {
  app->running = true;
  while (app->running) {
    wl_display_dispatch(app->wl_display);
  }
  // Cleanup
  swcl_array_free(&app->windows);
  wl_display_disconnect(app->wl_display);
  free(app);
}

void swcl_application_quit(SWCLApplication *app) { app->running = false; }

void swcl_application_set_cursor(SWCLApplication *app, const char *name,
                                 uint8_t size) {
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
