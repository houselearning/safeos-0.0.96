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
    int minimized;
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

// Window manager helpers
int window_count(void);
window_t* window_get(int idx);
void window_close(window_t *win);
void window_minimize(window_t *win);
void window_restore(window_t *win);
int window_is_minimized(window_t *win);
int window_is_visible(window_t *win);

// Process titlebar button clicks from desktop
int window_handle_title_bar_click(int x, int y); // returns 1 if event consumed

#endif
