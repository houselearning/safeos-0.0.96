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
window_t* window_create(int x, int y, int w, int h, const char *title);

// Draw a window
void window_draw(window_t *win);

// Handle events for a window
void window_handle_event(window_t *win, int event_type, int x, int y);

#endif
