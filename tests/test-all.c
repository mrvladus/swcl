#include "../src/swcl.h"
#include "tests.h"

void test_draw(SWCLWindow *win) {
  swcl_clear_background(0, 0, 1, 1);
  swcl_window_swap_buffers(win);
}

void test_pointer_enter(SWCLWindow *win, int x, int y) {
  int id = swcl_window_get_id(win);
  SWCL_LOG("Pointer entered: id=%d, x=%d, y=%d", id, x, y);
  // swcl_window_set_cursor(win, "left_ptr", 0);
}

void test_pointer_leave(SWCLWindow *win) {
  int id = swcl_window_get_id(win);
  SWCL_LOG("Pointer leave: id: %d", id);
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

  SWCL_LOG("Button pressed: id=%d, key=%d, state=%d, serial=%d, "
           "x=%d, y=%d",
           id, button, state, serial, swcl_cursor_pos.x, swcl_cursor_pos.y);

  // Handle window resize
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED) {
    uint8_t b = 10; // Border
    if (swcl_cursor_pos.x < b && swcl_cursor_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP_LEFT);
    else if (swcl_cursor_pos.x > width - b && swcl_cursor_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP_RIGHT);
    else if (swcl_cursor_pos.x < 5 && swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
    else if (swcl_cursor_pos.x > width - b && swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
    else if (swcl_cursor_pos.y < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_TOP);
    else if (swcl_cursor_pos.y > height - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_BOTTOM);
    else if (swcl_cursor_pos.x < b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_LEFT);
    else if (swcl_cursor_pos.x > width - b)
      swcl_window_resize(win, serial, SWCL_WINDOW_EDGE_RIGHT);
  }

  // Handle window drag
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED &&
      swcl_cursor_pos.y < 50 && swcl_cursor_pos.y > 5) {
    swcl_window_drag(win, serial);
  }
}

void test_kb_key(SWCLWindow *win, uint32_t key, SWCLButtonState state,
                 uint32_t serial) {
  SWCL_LOG("Key: keycode=%d, state=%d, serial=%d", key, state, serial);
}

// void test_kb_mod_key(SWCLWindow *win, uint32_t mods_depressed,
//                      uint32_t mods_latched, uint32_t mods_locked,
//                      uint32_t group, uint32_t serial) {
//   SWCL_LOG("Key: mods_depressed=%d, mods_latched=%d, "
//            "mods_locked=%d,group=%d,serial=%d",
//            mods_depressed, mods_latched, mods_locked, group, serial);
// }

SWCLTestResult test_all() {
  SWCLTestResult res = {0, 0};
  SWCLConfig swcl_cfg = {
      "io.github.mrvladus.Test",
      test_pointer_enter,
      test_pointer_leave,
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
      // .on_pointer_motion_cb = test_pointer_motion,
      // .on_mouse_scroll_cb = test_scroll,
      // .on_mouse_button_cb = test_mouse_button_pressed,
      // .on_keyboard_key_cb = test_kb_key,
      // .on_keyboard_mod_key_cb = test_kb_mod_key,
  };
  SWCLWindow *win = swcl_window_new(cfg);
  swcl_run();

  res.passed = 1;
  return res;
}
