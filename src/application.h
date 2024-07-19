#ifndef SWCL_APPLICATION_H
#define SWCL_APPLICATION_H

#include "swcl.h"
#include "xdg-shell-client-protocol.h"

#include <EGL/egl.h>
#include <wayland-client.h>

struct SWCLApplication {
  char *app_id;
  bool running;
  SWCLPoint cur_pos; // Cursor position
  int current_window_id;

  SWCLArray windows;

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
};

#endif
