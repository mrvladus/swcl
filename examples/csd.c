#include "../src/swcl-shapes.h"
#include "../src/swcl.h"

#define TITLEBAR_HEIGHT 30
#define BTN_RADIUS 10

// typedef struct {
//   SWCLColor color;
//   SWCLCircle circle;
// } TitleButton;

void pointer_enter(SWCLWindow *win, int x, int y) {
  swcl_application_set_cursor(win->app, "left_ptr", 16);
}

void mouse_button_pressed(SWCLWindow *win, SWCLMouseButton button,
                          SWCLButtonState state) {
  // Drag window if pressed on titlebar
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_PRESSED &&
      win->app->cursor_pos.y < TITLEBAR_HEIGHT &&
      win->app->cursor_pos.x < win->width - BTN_RADIUS * 6) {
    swcl_window_drag(win);
  }
  if (button == SWCL_MOUSE_1 && state == SWCL_BUTTON_RELEASED &&
      win->app->cursor_pos.y < TITLEBAR_HEIGHT) {
    // Handle close
    if (win->app->cursor_pos.x > win->width - BTN_RADIUS * 3) {
      swcl_application_quit(win->app);
    }
    // Handle maximize
    else if (win->app->cursor_pos.x > win->width - BTN_RADIUS * 6 &&
             win->app->cursor_pos.x < win->width - BTN_RADIUS * 3) {
      swcl_window_set_maximized(win, !win->maximized);
    }
  }
}

void draw_window_bg(SWCLWindow *win) {
  swcl_clear_background((SWCLColor){0, 0, 0, 0});
  swcl_draw_rounded_rect((SWCLColor){255, 255, 255, 255},
                         (SWCLRect){0, 0, win->width, win->height}, 12);
}

void draw_title_bar(SWCLWindow *win) {
  static const SWCLColor tb_color = {230, 230, 230, 255};
  static const SWCLColor close_color = {208, 114, 119, 255};
  static const SWCLColor max_color = {210, 183, 126, 255};
  // Draw background
  swcl_draw_rounded_rect(tb_color,
                         (SWCLRect){0, 0, win->width, TITLEBAR_HEIGHT}, 12);
  swcl_draw_rect(tb_color, (SWCLRect){0, 20, win->width, 10});
  // Draw close button
  swcl_draw_circle(close_color, (SWCLCircle){win->width - BTN_RADIUS * 2,
                                             TITLEBAR_HEIGHT / 2, BTN_RADIUS});
  // Draw maximize button
  swcl_draw_circle(max_color, (SWCLCircle){win->width - BTN_RADIUS * 5,
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
      .on_mouse_button_cb = mouse_button_pressed,
  };
  SWCLApplication *app = swcl_application_new(&cfg);
  SWCLWindow *win = swcl_window_new(app, "Rounded Window", 800, 600, 100, 100,
                                    false, false, draw);
  swcl_application_run(app);
  return 0;
}
