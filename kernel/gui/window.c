#include "window.h"
#include <stddef.h>

// Simple window manager backing store — minimal implementation
#define MAX_WINDOWS 32
static window_t windows[MAX_WINDOWS];
static int window_used[MAX_WINDOWS];

window_t* window_create(const char *title, int x, int y, int w, int h) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!window_used[i]) {
            window_used[i] = 1;
            windows[i].x = x;
            windows[i].y = y;
            windows[i].width = w;
            windows[i].height = h;
            windows[i].visible = 1;
            windows[i].title = title;
            return &windows[i];
        }
    }
    return NULL;
}

void window_begin_draw(window_t *win) {
    (void)win;
    // no-op for minimal implementation
}

void window_end_draw(window_t *win) {
    (void)win;
    // no-op for minimal implementation
}

void window_draw(window_t *win) {
    (void)win;
}

void window_handle_event(window_t *win, int event_type, int x, int y) {
    (void)win; (void)event_type; (void)x; (void)y;
}
