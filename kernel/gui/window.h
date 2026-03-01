#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

// Basic window structure
typedef struct window {
    int x;
    int y;
    int width;
    int height;
    int visible;
    const char *title;
} window_t;

// Create a window
// Note: many callers pass the title first ("Title", x, y, w, h)
window_t* window_create(const char *title, int x, int y, int w, int h);

// Begin drawing into a window (prepare clipping/backbuffer)
void window_begin_draw(window_t *win);

// Finish drawing a window (flush backbuffer)
void window_end_draw(window_t *win);

// Draw a window (legacy API)
void window_draw(window_t *win);

// Handle events for a window
void window_handle_event(window_t *win, int event_type, int x, int y);

#endif
