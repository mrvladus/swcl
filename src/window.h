#ifndef SWCL_WINDOW_H
#define SWCL_WINDOW_H

#include "application.h"
#include "swcl.h"

struct SWCLWindow {
  SWCLApplication *app;

  // Props
  int id;
  char *title;
  int width;
  int height;
  int min_width;
  int min_height;
  bool maximized;
  bool fullscreen;

  void (*on_draw_cb)(SWCLWindow *win);
  void (*on_pointer_enter_cb)(SWCLWindow *win, int x, int y);
  void (*on_pointer_leave_cb)(SWCLWindow *win);
  void (*on_pointer_motion_cb)(SWCLWindow *win, int x, int y);
  void (*on_mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*on_mouse_button_cb)(SWCLWindow *win, SWCLMouseButton button,
                             SWCLButtonState state, uint32_t serial);
  void (*on_keyboard_key_cb)(SWCLWindow *win, uint32_t key,
                             SWCLButtonState state, uint32_t serial);
  void (*on_keyboard_mod_key_cb)(SWCLWindow *win, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked,
                                 uint32_t group, uint32_t serial);

  // Wayland
  struct wl_surface *wl_surface;
  struct wl_callback *wl_callback;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;

  // EGL
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;
};

void swcl_window_make_current(SWCLWindow *win);

#endif
