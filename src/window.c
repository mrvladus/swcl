#define SWCL_ENABLE_DEBUG_LOGS
#include "window.h"
#include "application.h"
#include "swcl.h"

#include "xdg-shell-client-protocol.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>
#include <wayland-util.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------- UTILS -------- //

// Function to generate unique ID
static int swcl_generate_id() {
  static int id = -1; // static variable to hold the current ID value
  return ++id;        // increment and return the current ID
}

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

  if (win->egl_window) {
    win->width = (int)width;
    win->height = (int)height;
    glViewport(0, 0, width, height);
    wl_egl_window_resize(win->egl_window, width, height, 0, 0);
  }
}

static void on_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  SWCLWindow *win = data;
  win->app->running = false;
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

SWCLWindow *swcl_window_new(SWCLApplication *app, SWCLWindowConfig cfg) {
  SWCLWindow *win = (SWCLWindow *)malloc(sizeof(SWCLWindow));

  win->app = app;

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
  win->on_pointer_enter_cb = cfg.on_pointer_enter_cb;
  win->on_pointer_leave_cb = cfg.on_pointer_leave_cb;
  win->on_pointer_motion_cb = cfg.on_pointer_motion_cb;
  win->on_mouse_scroll_cb = cfg.on_mouse_scroll_cb;
  win->on_mouse_button_cb = cfg.on_mouse_button_cb;

  SWCL_LOG_DEBUG("Create new window with id: %d, width: %d, height: %d",
                 win->id, win->width, win->height);

  // ---------- WAYLAND STUFF ---------- //

  // Get wl_surface
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

  // ---------- EGL STUFF ---------- //

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

  // Enable multisampling for smoothing
  // glEnable(GL_MULTISAMPLE);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  swcl_window_make_current(win);
  win->on_draw_cb(win);

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

void swcl_window_make_current(SWCLWindow *win) {
  if (eglMakeCurrent(win->app->egl_display, win->egl_surface, win->egl_surface,
                     win->app->egl_context) == EGL_FALSE)
    SWCL_PANIC("Failed to make egl_surface current");
}

void swcl_window_drag(SWCLWindow *win, uint32_t serial) {
  xdg_toplevel_move(win->xdg_toplevel, win->app->wl_seat, serial);
}

void swcl_window_resize(SWCLWindow *win, uint32_t serial, SWCLWindowEdge edge) {
  xdg_toplevel_resize(win->xdg_toplevel, win->app->wl_seat, serial, edge);
}

void swcl_window_swap_buffers(SWCLWindow *win) {
  eglSwapBuffers(win->app->egl_display, win->egl_surface);
}

SWCLApplication *swcl_window_get_application(SWCLWindow *win) {
  return win->app;
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
