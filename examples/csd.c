#include "../src/swcl.h"

#define WINDOW_CORNER_RADIUS 10
#define TITLEBAR_HEIGHT 30
#define BTN_RADIUS 10

// Set arrow cursor on pointer entering the window
void pointer_enter(SWCLWindow *win, int x, int y) {
  swcl_application_set_cursor(win->app, "left_ptr", 16);
}

// Drag window if pressed on titlebar
void handle_drag(SWCLWindow *win) {
  if (win->app->cursor_pos.y < TITLEBAR_HEIGHT &&
      win->app->cursor_pos.x < win->width - BTN_RADIUS * 9) {
    swcl_window_drag(win);
  }
}

// Resize window if cliked on the window edges
void handle_resize(SWCLWindow *win) {
  static uint8_t b = 5; // Border size
  if (win->app->cursor_pos.x < b && win->app->cursor_pos.y < b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_LEFT);
  else if (win->app->cursor_pos.x > win->width - b &&
           win->app->cursor_pos.y < b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP_RIGHT);
  else if (win->app->cursor_pos.x < 5 &&
           win->app->cursor_pos.y > win->height - b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_LEFT);
  else if (win->app->cursor_pos.x > win->width - b &&
           win->app->cursor_pos.y > win->height - b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM_RIGHT);
  else if (win->app->cursor_pos.y < b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_TOP);
  else if (win->app->cursor_pos.y > win->height - b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_BOTTOM);
  else if (win->app->cursor_pos.x < b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_LEFT);
  else if (win->app->cursor_pos.x > win->width - b)
    swcl_window_resize(win, SWCL_WINDOW_EDGE_RIGHT);
}

// Handler for 3 title buttons
void handle_buttons(SWCLWindow *win) {
  uint32_t x = win->app->cursor_pos.x;
  uint32_t y = win->app->cursor_pos.y;
  if (y < TITLEBAR_HEIGHT) {
    // Handle close
    if (x > win->width - BTN_RADIUS * 3) {
      swcl_application_quit(win->app);
    }
    // Handle maximize
    else if (x > win->width - BTN_RADIUS * 6 &&
             x < win->width - BTN_RADIUS * 3) {
      swcl_window_set_maximized(win, !win->maximized);
    }
    // Handle minimize
    else if (x > win->width - BTN_RADIUS * 9 &&
             x < win->width - BTN_RADIUS * 7) {
      swcl_window_minimize(win);
    }
  }
}

// Callback for mouse button event
void mouse_button_pressed(SWCLWindow *win, SWCLMouseButton button,
                          SWCLButtonState state) {
  if (button == SWCL_MOUSE_1) {
    if (state == SWCL_BUTTON_PRESSED) {
      handle_resize(win);
      handle_drag(win);
    } else {
      handle_buttons(win);
    }
  }
}

// Set cursor for each edge and corner
void handle_resize_cursor(SWCLWindow *win, int x, int y) {
  static uint8_t b = 5;     // Border size
  static uint8_t size = 16; // Cursor size
  if (x < b && y < b)
    swcl_application_set_cursor(win->app, "top_left_corner", size);
  else if (x > win->width - b && y < b)
    swcl_application_set_cursor(win->app, "top_right_corner", size);
  else if (x < b && y > win->height - b)
    swcl_application_set_cursor(win->app, "bottom_left_corner", size);
  else if (x > win->width - b && y > win->height - b)
    swcl_application_set_cursor(win->app, "bottom_right_corner", size);
  else if (y < b)
    swcl_application_set_cursor(win->app, "top_side", size);
  else if (y > win->height - b)
    swcl_application_set_cursor(win->app, "bottom_side", size);
  else if (x < b)
    swcl_application_set_cursor(win->app, "left_side", size);
  else if (x > win->width - b)
    swcl_application_set_cursor(win->app, "right_side", size);
  else
    swcl_application_set_cursor(win->app, "left_ptr", size);
}

// Callback for pointer motion event
void pointer_motion(SWCLWindow *win, int x, int y) {
  handle_resize_cursor(win, x, y);
}

// Draw rounded window
void draw_window_bg(SWCLWindow *win) {
  swcl_clear_background((SWCLColor){0, 0, 0, 0});
  swcl_draw_rounded_rect((SWCLColor){255, 255, 255, 255},
                         (SWCLRect){0, 0, win->width, win->height},
                         WINDOW_CORNER_RADIUS);
}

// Draw title bar with buttons
void draw_title_bar(SWCLWindow *win) {
  // Draw background
  swcl_draw_rounded_rect((SWCLColor){230, 230, 230, 255},
                         (SWCLRect){0, 0, win->width, TITLEBAR_HEIGHT},
                         WINDOW_CORNER_RADIUS);
  swcl_draw_rect((SWCLColor){230, 230, 230, 255},
                 (SWCLRect){0, TITLEBAR_HEIGHT - WINDOW_CORNER_RADIUS,
                            win->width, WINDOW_CORNER_RADIUS});
  // Draw close button
  swcl_draw_circle((SWCLColor){208, 114, 119, 255},
                   (SWCLCircle){win->width - BTN_RADIUS * 2,
                                TITLEBAR_HEIGHT / 2, BTN_RADIUS});
  // Draw maximize button
  swcl_draw_circle((SWCLColor){210, 183, 126, 255},
                   (SWCLCircle){win->width - BTN_RADIUS * 5,
                                TITLEBAR_HEIGHT / 2, BTN_RADIUS});
  // Draw minimize button
  swcl_draw_circle((SWCLColor){154, 184, 123, 255},
                   (SWCLCircle){win->width - BTN_RADIUS * 8,
                                TITLEBAR_HEIGHT / 2, BTN_RADIUS});
}

void draw(SWCLWindow *win) {
  draw_window_bg(win);
  draw_title_bar(win);
  swcl_window_swap_buffers(win);
}

int main() {
  SWCLConfig cfg = {
      .app_id = "io.github.mrvladus.Test",
      .on_pointer_enter_cb = pointer_enter,
      .on_pointer_motion_cb = pointer_motion,
      .on_mouse_button_cb = mouse_button_pressed,
  };
  SWCLApplication *app = swcl_application_new(&cfg);
  SWCLWindow *win = swcl_window_new(app, "Client-Side Decorations", 800, 600,
                                    100, 100, false, false, draw);
  swcl_application_run(app);
  return 0;
}
