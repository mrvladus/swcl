#define SWCL_IMPLEMENTATION
#include "../swcl.h"
#include <GL/gl.h>

// Drawing function that called each frame
void draw(SWCLWindow *win) {
  // Draw white background
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  // Swap buffers
  swcl_window_swap_buffers(win);
}

int main() {
  // Create basic config
  SWCLConfig cfg = {.app_id = "io.github.mrvladus.Test"};
  // Create application
  SWCLApplication *app = swcl_application_new(&cfg);
  // Create window
  SWCLWindow *win = swcl_window_new(app, "Basic Window", 800, 600, 100, 100,
                                    false, false, draw);
  swcl_window_show(win);
  // Run application
  swcl_application_run(app);
  return 0;
}
