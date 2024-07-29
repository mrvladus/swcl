#include "../src/swcl.h"
#include <stdint.h>

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
  SWCL_LOG("Pointer leave: id: %d", id);
}

void test_pointer_motion(SWCLWindow *win, int x, int y) {
  static uint8_t border = 5;

  if (x < border && y < border)
    swcl_set_cursor("top_left_corner", 16);
  else if (x > win->width - border && y < border)
    swcl_set_cursor("top_right_corner", 16);
  else if (x < border && y > win->height - border)
    swcl_set_cursor("bottom_left_corner", 16);
  else if (x > win->width - border && y > win->height - border)
    swcl_set_cursor("bottom_right_corner", 16);
  else if (y < border)
    swcl_set_cursor("top_side", 16);
  else if (y > win->height - border)
    swcl_set_cursor("bottom_side", 16);
  else if (x < border)
    swcl_set_cursor("left_side", 16);
  else if (x > win->width - border)
    swcl_set_cursor("right_side", 16);
  else
    swcl_set_cursor("left_ptr", 16);

  SWCL_LOG("Pointer motion: id=%d, x=%d, y=%d", swcl_window_get_id(win), x, y);
}

void test_scroll(SWCLWindow *win, SWCLScrollDirection dir) {
  if (dir == SWCL_SCROLL_UP)
    SWCL_LOG("Scroll UP");
  else
    SWCL_LOG("Scroll DOWN");
}

void test_mouse_button_pressed(SWCLWindow *win, SWCLMouseButton button,
                               SWCLButtonState state) {
  int id = swcl_window_get_id(win);
  int width = swcl_window_get_width(win);
  int height = swcl_window_get_height(win);

  SWCL_LOG("Button pressed: id=%d, key=%d, state=%d, "
           "x=%d, y=%d",
           id, button, state, swcl_cursor_pos.x, swcl_cursor_pos.y);

  // Handle window resize
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED) {
    uint8_t b = 10; // Border
    if (swcl_cursor_pos.x < b && swcl_cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_LEFT);
    else if (swcl_cursor_pos.x > width - b && swcl_cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_RIGHT);
    else if (swcl_cursor_pos.x < 5 && swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
    else if (swcl_cursor_pos.x > width - b && swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
    else if (swcl_cursor_pos.y < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP);
    else if (swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM);
    else if (swcl_cursor_pos.x < b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_LEFT);
    else if (swcl_cursor_pos.x > width - b)
      swcl_window_resize(win, SWCL_WINDOW_EDGE_RIGHT);
  }

  // Handle window drag
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED &&
      swcl_cursor_pos.y < 50 && swcl_cursor_pos.y > 5) {
    swcl_window_drag(win);
  }
}

void test_kb_key(SWCLWindow *win, uint32_t key, SWCLButtonState state,
                 uint32_t serial) {
  SWCL_LOG("Key: keycode=%d, state=%d, serial=%d", key, state, serial);

  // Quit when Esc released
  if (key == 1 && state == 0) {
    swcl_quit();
  }
}

// void test_kb_mod_key(SWCLWindow *win, uint32_t mods_depressed,
//                      uint32_t mods_latched, uint32_t mods_locked,
//                      uint32_t group, uint32_t serial) {
//   SWCL_LOG("Key: mods_depressed=%d, mods_latched=%d, "
//            "mods_locked=%d,group=%d,serial=%d",
//            mods_depressed, mods_latched, mods_locked, group, serial);
// }

int main() {
  SWCLConfig swcl_cfg = {
      "io.github.mrvladus.Test",
      test_pointer_enter,
      test_pointer_leave,
      test_pointer_motion,
      test_mouse_button_pressed,
      test_scroll,
      test_kb_key,
  };
  swcl_init(&swcl_cfg);
  SWCLWindowConfig cfg = {
      .title = "Test",
      .height = 700,
      .width = 1000,
      .min_height = 100,
      .min_width = 100,
      // .maximized = true,
      // .fullscreen = true,
      .on_draw_cb = test_draw,
  };
  SWCLWindow *win = swcl_window_new(cfg);
  swcl_run();
  return 0;
}
