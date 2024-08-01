#include "../src/swcl.h"
#include <stdint.h>

void draw(SWCLWindow *win) {
  swcl_clear_background(0, 0, 1, 1);
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
           id, button, state, swcl_cursor_pos.x, swcl_cursor_pos.y);

  // Handle window resize
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED) {
    uint8_t b = 5; // Border size
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

void kb_key(SWCLWindow *win, uint32_t key, SWCLButtonState state,
            uint32_t serial) {
  SWCL_LOG("Key: keycode=%d, state=%d, serial=%d", key, state, serial);

  // Quit when Esc released
  if (key == 1 && state == 0) {
    swcl_quit();
  }
}

void kb_mod_key(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched,
                uint32_t mods_locked, uint32_t group) {
  SWCL_LOG("Key: mods_depressed=%d, mods_latched=%d, "
           "mods_locked=%d, group=%d",
           mods_depressed, mods_latched, mods_locked, group);
}

int main() {
  SWCLConfig swcl_cfg = {
      "io.github.mrvladus.Test", pointer_enter, pointer_leave, pointer_motion,
      mouse_button_pressed,      scroll,        kb_key,        kb_mod_key,
  };
  swcl_init(&swcl_cfg);
  SWCLWindow *win =
      swcl_window_new("Example Window", 800, 600, 100, 100, false, false, draw);
  swcl_window_ancor(SWCL_ANCOR_NONE);
  swcl_window_request_ssr(win);
  swcl_run();
  return 0;
}
