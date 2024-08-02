#include "../src/swcl.h"

#include <GL/gl.h>

void draw(SWCLWindow *win) {
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  // Draw the rectangle
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // Red color
  glBegin(GL_QUADS);                 // Begin drawing quads
  glVertex2f(-0.5f, -0.5f);          // Bottom-left vertex
  glVertex2f(0.5f, -0.5f);           // Bottom-right vertex
  glVertex2f(0.5f, 0.5f);            // Top-right vertex
  glVertex2f(-0.5f, 0.5f);           // Top-left vertex
  glEnd();                           // End drawing quads
  swcl_window_swap_buffers(win);
}

int main() {
  SWCLConfig swcl_cfg = {"io.github.mrvladus.Test"};
  swcl_init(&swcl_cfg);
  SWCLWindow *win =
      swcl_window_new("Example Window", 800, 600, 100, 100, false, false, draw);
  swcl_run();
  return 0;
}
