#ifndef SWCL_H
#define SWCL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -------- MACROS -------- //

// Logging
#define SWCL_LOG(format, ...)                                                  \
  fprintf(stderr, "SWCL: " format "\n", ##__VA_ARGS__)

#ifdef SWCL_ENABLE_DEBUG_LOGS
#define SWCL_LOG_DEBUG(format, ...)                                            \
  fprintf(stdout, "SWCL DEBUG: " format "\n", ##__VA_ARGS__)
#else
#define SWCL_LOG_DEBUG(format, ...) ((void)0)
#endif

#define SWCL_PANIC(format, ...)                                                \
  do {                                                                         \
    fprintf(stderr, "SWCL PANIC: " format "\n", ##__VA_ARGS__);                \
    exit(1);                                                                   \
  } while (0)

// ---------- UTILS ---------- //

// Simple dynamic array structure
typedef struct {
  int length;
  int capacity;
  void **items;
} SWCLArray;

// Create new dynamic array with given initial capacity.
SWCLArray swcl_array_new(int initial_capacity);

// Add item to the end of the array, resizing it if needed.
void swcl_array_append(SWCLArray *array, void *item);

// Remove item from the array
void swcl_array_remove(SWCLArray *array, void *item);

// Destroy array
void swcl_array_free(SWCLArray *array);

// ---------- ENUMS ---------- //

// Direction of the mouse scroll wheel
typedef enum {
  SWCL_SCROLL_UP = 0,
  SWCL_SCROLL_DOWN = 1,
} SWCLScrollDirection;

// Mouse buttons keycodes
typedef enum {
  SWCL_MOUSE_1 = 272, // Left
  SWCL_MOUSE_2 = 273, // Right
  SWCL_MOUSE_3 = 274, // Middle
  SWCL_MOUSE_4 = 275, // Back
  SWCL_MOUSE_5 = 276, // Forward
} SWCLMouseButton;

// State of the button
typedef enum {
  SWCL_BUTTON_RELEASED = 0,
  SWCL_BUTTON_PRESSED = 1,
} SWCLButtonState;

// Edge or corner of the window. Passed to 'swcl_window_resize' function.
typedef enum {
  SWCL_WINDOW_EDGE_NONE = 0,
  SWCL_WINDOW_EDGE_TOP = 1,
  SWCL_WINDOW_EDGE_BOTTOM = 2,
  SWCL_WINDOW_EDGE_LEFT = 4,
  SWCL_WINDOW_EDGE_TOP_LEFT = 5,
  SWCL_WINDOW_EDGE_BOTTOM_LEFT = 6,
  SWCL_WINDOW_EDGE_RIGHT = 8,
  SWCL_WINDOW_EDGE_TOP_RIGHT = 9,
  SWCL_WINDOW_EDGE_BOTTOM_RIGHT = 10,
} SWCLWindowEdge;

// ---------- STRUCTS ---------- //

// Position with x and y coordinates
typedef struct {
  int x;
  int y;
} SWCLPoint;

// ---------- APPLICATON ---------- //

// Application object. Must be created first.
typedef struct SWCLApplication SWCLApplication;

// Creates new SWCLApplication.
// It will initialize connection to Wayland display, register mouse, keyboard
// and touch devices. Also it will initialize EGL and create OpenGL context.
SWCLApplication *swcl_application_new(char *app_id);

// Start the application loop
void swcl_application_run(SWCLApplication *app);

SWCLPoint swcl_application_get_cursor_position(SWCLApplication *app);

// ---------- WINDOW ---------- //

// Toplevel window object
typedef struct SWCLWindow SWCLWindow;

typedef struct {
  // Properties
  char *title;
  int width;
  int height;
  int min_width;
  int min_height;
  bool maximized;
  bool fullscreen;

  // Callbacks
  void (*on_draw_cb)(SWCLWindow *win);
  void (*on_pointer_enter_cb)(SWCLWindow *win, int x, int y);
  void (*on_pointer_leave_cb)(SWCLWindow *win);
  void (*on_pointer_motion_cb)(SWCLWindow *win, int x, int y);
  void (*on_mouse_scroll_cb)(SWCLWindow *win, SWCLScrollDirection dir);
  void (*on_mouse_button_cb)(SWCLWindow *win, SWCLMouseButton button,
                             SWCLButtonState state, uint32_t serial);
} SWCLWindowConfig;

// Create new window.
// This function takes care of creating native wayland window with stuff like
// wl_surface, xdg_surface, xdg_toplevel and putting egl_window with OpenGL
// context into it.
SWCLWindow *swcl_window_new(SWCLApplication *app, SWCLWindowConfig cfg);

// Connect callback to window event.
// First argument is event name.
// Second argument is callback funtion.
// Here is the event names and their arguments passed to callbacks:
//
// "draw" -> (SWCLWindow *win)
//
// "pointer-enter" -> (SWCLWindow *win, int x, int y)
//
// "pointer-leave" -> (SWCLWindow *win)
//
// "pointer-motion" -> (SWCLWindow *win, int x, int y)
//
// "mouse-scroll" -> (SWCLWindow *win, SWCLScrollDirection dir)
//
// "mouse-button" -> (SWCLWindow *win, SWCLMouseButton button, SWCLButtonState
// state, uint32_t serial)
void swcl_window_connect(SWCLWindow *win, void *callback);

// Tells compositor to begin native drag operation. With this window can be
// snapped to the sides if comositor allows it. This function is useful for
// implementing Client-Side Decorations (CSD).
//
// serial - is event id. It can be taken from "mouse-button" callback.
void swcl_window_drag(SWCLWindow *win, uint32_t serial);

// Tells compositor to begin native resize operation. With this window can be
// resized if comositor allows it.
// This function is useful for implementing Client-Side Decorations (CSD).
//
// serial - is event id. It can be taken from "mouse-button" callback.
void swcl_window_resize(SWCLWindow *win, SWCLWindowEdge edge, uint32_t serial);

// Swap OpenGL buffer for rendered frame
void swcl_window_swap_buffers(SWCLWindow *win);

// Get window properties

// Application object
SWCLApplication *swcl_window_get_application(SWCLWindow *win);

// ID property
int swcl_window_get_id(SWCLWindow *win);

// Title property
char *swcl_window_get_title(SWCLWindow *win);

// Width property
int swcl_window_get_width(SWCLWindow *win);

// Height property
int swcl_window_get_height(SWCLWindow *win);

// Minimum width property
int swcl_window_get_min_width(SWCLWindow *win);

// Minimum height property
int swcl_window_get_min_height(SWCLWindow *win);

// Maximized state property
bool swcl_window_get_maximized(SWCLWindow *win);

// Fulscreen state property
bool swcl_window_get_fullscreen(SWCLWindow *win);

// Set window properties

// Set window title
void swcl_window_set_title(SWCLWindow *win, char *title);

// Set window size
void swcl_window_set_min_size(SWCLWindow *win, int width, int height);

// Set window minimum size
void swcl_window_set_min_size(SWCLWindow *win, int min_width, int min_height);

// Set window maximized state
void swcl_window_set_maximized(SWCLWindow *win, bool maximized);

// Set window fullscreen state
void swcl_window_set_fullscreen(SWCLWindow *win, bool maximized);

// ---------- DRAWING ---------- //

// Clear window with RGBA color
void swcl_clear_background(float r, float g, float b, float a);

#endif // SWCL_H
