#include "../src/swcl-shapes.h"
#include "../src/swcl.h"

#include <GL/gl.h>

// Set cursor when mouse hovers over the window
void pointer_enter(SWCLWindow *win, int x, int y) {
  swcl_application_set_cursor(win->app, "left_ptr", 16);
}

// Drawing function
void draw(SWCLWindow *win) {
  // Make transparent background
  swcl_clear_background((SWCLColor){0, 0, 0, 0});
  // Draw rounded rect for window background
  swcl_draw_rounded_rect((SWCLColor){255, 255, 255, 255},
                         (SWCLRect){0, 0, win->width, win->height}, 12);
  // Swap buffers
  swcl_window_swap_buffers(win);
}

int main() {
  SWCLConfig cfg = {
      .app_id = "io.github.mrvladus.Test",
      .on_pointer_enter_cb = pointer_enter,
  };
  SWCLApplication *app = swcl_application_new(&cfg);
  SWCLWindow *win = swcl_window_new(app, "Rounded Window", 800, 600, 100, 100,
                                    false, false, draw);
  swcl_application_run(app);
  return 0;
}
