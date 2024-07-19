#include "swcl.h"

#include <GLES2/gl2.h>

void swcl_clear_background(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT);
}
