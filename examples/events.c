#include "../src/swcl.h"
#include <GL/gl.h>

void draw(SWCLWindow *win) {
  glClearColor(0, 0, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  swcl_window_swap_buffers(win);
}

void pointer_enter(SWCLWindow *win, int x, int y) {
  SWCL_LOG("Pointer entered: id=%d, x=%d, y=%d", win->id, x, y);
}

void pointer_leave(SWCLWindow *win) {
  SWCL_LOG("Pointer leave: id: %d", win->id);
}

void pointer_motion(SWCLWindow *win, int x, int y) {
  SWCL_LOG("Pointer motion: id=%d, x=%d, y=%d", win->id, x, y);
}

void scroll(SWCLWindow *win, SWCLScrollDirection dir) {
  if (dir == SWCL_SCROLL_UP)
    SWCL_LOG("Scroll UP");
  else
    SWCL_LOG("Scroll DOWN");
}

void mouse_button_pressed(SWCLWindow *win, SWCLMouseButton button,
                          SWCLButtonState state) {
  SWCL_LOG("Button pressed: id=%d, key=%d, state=%d, "
           "x=%d, y=%d",
           win->id, button, state, win->app->cursor_pos.x,
           win->app->cursor_pos.y);
}

void kb_key(SWCLWindow *win, uint32_t key, SWCLButtonState state) {
  SWCL_LOG("Key: keycode=%d, state=%d", key, state);
}

void kb_mod_key(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched,
                uint32_t mods_locked, uint32_t group) {
  SWCL_LOG("Key: mods_depressed=%d, mods_latched=%d, "
           "mods_locked=%d, group=%d",
           mods_depressed, mods_latched, mods_locked, group);
}

int main() {
  SWCLConfig cfg = {
      .app_id = "io.github.mrvladus.Test",
      .on_pointer_enter_cb = pointer_enter,
      .on_pointer_leave_cb = pointer_leave,
      .on_pointer_motion_cb = pointer_motion,
      .on_mouse_button_cb = mouse_button_pressed,
      .on_mouse_scroll_cb = scroll,
      .on_keyboard_key_cb = kb_key,
      .on_keyboard_mod_key_cb = kb_mod_key,
  };
  SWCLApplication *app = swcl_application_new(&cfg);
  SWCLWindow *win = swcl_window_new(app, "Window Events", 800, 600, 100, 100,
                                    false, false, draw);
  swcl_application_run(app);
  return 0;
}
