// Include implementation in ONE source file.
// In other file just include swcl.h
#define SWCL_ENABLE_DEBUG_LOGS
#define SWCL_IMPLEMENTATION
#include "swcl.h"

// ---------- EVENT CALLBACKS ---------- //

// This function is triggered when pointer enters the window.
void pointer_enter_cb(SWCLWindow *win, size_t x, size_t y) {
  // We need to set cursor when pointer enters window, otherwise it won't be changed.
  swcl_application_set_cursor(win->app, "left_ptr", 16);
  printf("Pointer entered at x=%zu, y=%zu\n", x, y);
}

// In this function we can track pointer position over window.
void pointer_motion_cb(SWCLWindow *win, size_t x, size_t y) { printf("Pointer motion at x=%zu, y=%zu\n", x, y); }

// This function is triggered when pointer leaves the window.
void pointer_leave_cb(SWCLWindow *win) { printf("Pointer leave\n"); }

// Mouse button events inside the window callback
void mouse_button_cb(SWCLWindow *win, SWCLMouseButton btn, SWCLButtonState state) {
  printf("Mouse button %d %s\n", btn, state == SWCL_BUTTON_PRESSED ? "pressed" : "released");

  // Get mouse position
  size_t x = win->app->cursor_pos_x;
  size_t y = win->app->cursor_pos_y;

  // Drag window if left mouse button is clicked and mouse Y position is less than 30px from the top of the window.
  if (btn == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED && y < 30) swcl_window_drag(win);

  // Resize window if clicked on the window edges
  size_t border = 10; // Edge click area size
  if (x < border && y < border) swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_LEFT);
  else if (x > win->width - border && y < border) swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_RIGHT);
  else if (x < border && y > win->height - border) swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
  else if (x > win->width - border && y > win->height - border) swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
  else if (y < border) swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP);
  else if (y > win->height - border) swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM);
  else if (x < border) swcl_window_resize(win, SWCL_WINDOW_EDGE_LEFT);
  else if (x > win->width - border) swcl_window_resize(win, SWCL_WINDOW_EDGE_RIGHT);
}

// Scroll event inside the window callback
void mouse_scroll_cb(SWCLWindow *win, SWCLScrollDirection dir) {
  printf("Scroll %s\n", dir == SWCL_SCROLL_DOWN ? "down" : "up");
}

// Keyboard key events inside the window callback
void keyboard_key_cb(SWCLWindow *win, size_t key, SWCLButtonState state) {
  printf("Keyboard key %zu %s\n", key, state == SWCL_BUTTON_PRESSED ? "pressed" : "released");

  // Close application if ESC pressed
  if (key == 1 && state == SWCL_BUTTON_PRESSED) swcl_application_quit(win->app);
}

// Keyboard modifiers events inside the window callback
void keyboard_mod_key_cb(SWCLWindow *win, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
                         uint32_t group) {
  printf("Key: mods_depressed=%d, mods_latched=%d, mods_locked=%d, group=%d\n", mods_depressed, mods_latched,
         mods_locked, group);
}

// Draw window callback. Called every frame.
void draw_window_cb(SWCLWindow *win) {
  // We will use OpenGL directly here but libraries can be used like rlgl.h
  // Draw white background
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  // Swap buffers. This function needs to be called at the end.
  swcl_window_swap_buffers(win);
}

// ---------- MAIN ---------- //

int main(int argc, char *argv[]) {
  // Create application and provide callbacks for events
  SWCLApplication *app = swcl_application_new("com.example.App", pointer_enter_cb, pointer_motion_cb, pointer_leave_cb,
                                              mouse_button_cb, mouse_scroll_cb, keyboard_key_cb, keyboard_mod_key_cb);
  // Create window
  SWCLWindow *win = swcl_window_new(app, "Hello SWCL!", 800, 600, 100, 100, false, false, draw_window_cb);
  swcl_window_show(win);
  // Run application
  swcl_application_run(app);
  return 0;
}
