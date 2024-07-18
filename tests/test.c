#include "../src/swcl.h"

void test_draw(SWCLWindow *win) {
  swcl_clear_background(0, 0, 1, 1);
  swcl_window_swap_buffers(win);
}

void test_pointer_enter(SWCLWindow *win, int x, int y) {
  int id = swcl_window_get_id(win);
  SWCL_LOG("Pointer entered: id=%d, x=%d, y=%d", id, x, y);
}

void test_pointer_leave(SWCLWindow *win) {
  int id = swcl_window_get_id(win);
  SWCL_LOG("Pointer leaved: id: %d", id);
}

void test_pointer_motion(SWCLWindow *win, int x, int y) {
  int id = swcl_window_get_id(win);
  SWCL_LOG("Pointer motion: id=%d, x=%d, y=%d", id, x, y);
}

void test_scroll(SWCLWindow *win, SWCLScrollDirection dir) {
  if (dir == SWCL_SCROLL_UP)
    SWCL_LOG("Scroll UP");
  else
    SWCL_LOG("Scroll DOWN");
}

void test_mouse_button_pressed(SWCLWindow *win, SWCLMouseButton button,
                               SWCLButtonState state, uint32_t serial) {
  int id = swcl_window_get_id(win);
  int width = swcl_window_get_width(win);
  int height = swcl_window_get_height(win);
  SWCLApplication *app = swcl_window_get_application(win);
  SWCLPoint cur_pos = swcl_application_get_cursor_position(app);

  SWCL_LOG("Button pressed: id=%d, key=%d, state=%d, serial=%d, "
           "x=%d, y=%d",
           id, button, state, serial, cur_pos.x, cur_pos.y);

  // Handle window resize
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED) {
    uint8_t b = 10; // Border
    if (cur_pos.x < b && cur_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP_LEFT);
    else if (cur_pos.x > width - b && cur_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP_RIGHT);
    else if (cur_pos.x < 5 && cur_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
    else if (cur_pos.x > width - b && cur_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
    else if (cur_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP);
    else if (cur_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM);
    else if (cur_pos.x < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_LEFT);
    else if (cur_pos.x > width - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_RIGHT);
  }

  // Handle window drag
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED &&
      cur_pos.y < 50 && cur_pos.y > 5) {
    swcl_window_drag(win, serial);
  }
}

int main(int argc, char const *argv[]) {
  SWCLApplication *app = swcl_application_new("io.github.mrvladus.Test");
  SWCLWindowConfig cfg = {
      .title = "Test",
      .height = 700,
      .width = 1000,
      .min_height = 100,
      .min_width = 100,
      // .maximized = true,
      // .fullscreen = true,
      .on_draw_cb = test_draw,
      .on_pointer_enter_cb = test_pointer_enter,
      .on_pointer_leave_cb = test_pointer_leave,
      .on_pointer_motion_cb = test_pointer_motion,
      .on_mouse_scroll_cb = test_scroll,
      .on_mouse_button_cb = test_mouse_button_pressed,
  };
  SWCLWindow *win = swcl_window_new(app, cfg);

  swcl_application_run(app);
  return 0;
}
