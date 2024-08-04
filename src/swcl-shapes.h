

#ifndef SWCL_SHAPES_H
#define SWCL_SHAPES_H

#include "swcl.h"
#include <GL/gl.h>

#include <math.h>

#include <stdint.h>

// ---------- STRUCTS ---------- //

// RGBA color. Values can be from 0 to 255.
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} SWCLColor;

// Rectangle
typedef struct {
  int x;
  int y;
  int w;
  int h;
} SWCLRect;

// Circle where 'cx' and 'cy' are coordinates of the center of the circle and
// 'r' is the radius
typedef struct {
  int cx;
  int cy;
  int r;
} SWCLCircle;

static inline void swcl_draw_rect(SWCLColor color, SWCLRect rect);
static inline void swcl_draw_circle(SWCLColor color, SWCLCircle circle);
// ---------- PRIVATE FUNCTIONS ---------- //

// Setup coordinates system. If window size is not changed - do nothing.
// Don't use directly.
static inline void __swcl_set_orthographic_projection() {
  static uint32_t width;
  static uint32_t height;
  GLint viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  if (width && height && viewport[2] == width && viewport[3] == height)
    return;

  width = viewport[2];
  height = viewport[3];

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// ---------- PUBLIC FUNCTIONS ---------- //

static inline void swcl_clear_background(SWCLColor color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

// Draw rectangle with given color and dimentions.
static inline void swcl_draw_rect(SWCLColor color, SWCLRect rect) {
  __swcl_set_orthographic_projection();
  glColor4ub(color.r, color.g, color.b, color.a);
  glBegin(GL_QUADS);
  glVertex2i(rect.x, rect.y);
  glVertex2i(rect.x + rect.w, rect.y);
  glVertex2i(rect.x + rect.w, rect.y + rect.h);
  glVertex2i(rect.x, rect.y + rect.h);
  glEnd();
}

static inline void swcl_draw_rounded_rect(SWCLColor color, SWCLRect rect,
                                          int radius) {
  // Draw corner circles
  swcl_draw_circle(color,
                   (SWCLCircle){rect.x + radius, rect.y + radius, radius});
  swcl_draw_circle(
      color, (SWCLCircle){rect.x + rect.w - radius, rect.y + radius, radius});
  swcl_draw_circle(
      color, (SWCLCircle){rect.x + radius, rect.y + rect.h - radius, radius});
  swcl_draw_circle(color, (SWCLCircle){rect.x + rect.w - radius,
                                       rect.y + rect.h - radius, radius});
  // Draw rects
  swcl_draw_rect(
      color, (SWCLRect){rect.x + radius, rect.y, rect.w - radius * 2, rect.h});
  swcl_draw_rect(
      color, (SWCLRect){rect.x, rect.y + radius, rect.w, rect.h - radius * 2});
}

void swcl_draw_circle(SWCLColor color, SWCLCircle circle) {
  __swcl_set_orthographic_projection();
  glColor4ub(color.r, color.g, color.b, color.a);
  glBegin(GL_POLYGON);
  for (int i = 0; i <= 100; i++) {
    double angle = 2 * M_PI * i / 100;
    double x = cos(angle) * circle.r;
    double y = sin(angle) * circle.r;
    glVertex2d(circle.cx + x, circle.cy + y);
  }
  glEnd();
}
#endif
