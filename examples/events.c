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
  static uint8_t border = 5;

  if (x < border && y < border)
    swcl_application_set_cursor(win->app, "top_left_corner", 16);
  else if (x > win->width - border && y < border)
    swcl_application_set_cursor(win->app, "top_right_corner", 16);
  else if (x < border && y > win->height - border)
    swcl_application_set_cursor(win->app, "bottom_left_corner", 16);
  else if (x > win->width - border && y > win->height - border)
    swcl_application_set_cursor(win->app, "bottom_right_corner", 16);
  else if (y < border)
    swcl_application_set_cursor(win->app, "top_side", 16);
  else if (y > win->height - border)
    swcl_application_set_cursor(win->app, "bottom_side", 16);
  else if (x < border)
    swcl_application_set_cursor(win->app, "left_side", 16);
  else if (x > win->width - border)
    swcl_application_set_cursor(win->app, "right_side", 16);
  else
    swcl_application_set_cursor(win->app, "left_ptr", 16);

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
  int id = win->id;
  int width = win->width;
  int height = win->height;

  SWCL_LOG("Button pressed: id=%d, key=%d, state=%d, "
           "x=%d, y=%d",
           id, button, state, win->app->cursor_pos.x, win->app->cursor_pos.y);

  // Handle window resize
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED) {
    uint8_t b = 5; // Border size
    if (win->app->cursor_pos.x < b && win->app->cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_LEFT);
    else if (win->app->cursor_pos.x > width - b && win->app->cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_RIGHT);
    else if (win->app->cursor_pos.x < 5 && win->app->cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
    else if (win->app->cursor_pos.x > width - b &&
             win->app->cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
    else if (win->app->cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP);
    else if (win->app->cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM);
    else if (win->app->cursor_pos.x < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_LEFT);
    else if (win->app->cursor_pos.x > width - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_RIGHT);
  }

  // Handle window drag
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED &&
      win->app->cursor_pos.y < 50 && win->app->cursor_pos.y > 5) {
    swcl_window_drag(win);
  }
}

void kb_key(SWCLWindow *win, uint32_t key, SWCLButtonState state) {
  SWCL_LOG("Key: keycode=%d, state=%d", key, state);

  // Quit when Esc released
  if (key == 1 && state == 0) {
    swcl_application_quit(win->app);
  }
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
