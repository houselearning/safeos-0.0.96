#include "window.h"
#include "gui.h"
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
    if (!win->visible) return;

    // Draw window border
    gui_draw_rect(win->x, win->y, win->width, win->height, 0xFFFFFF);

    // Title bar
    gui_draw_rect(win->x + 2, win->y + 2, win->width - 4, 20, 0xC0C0C0);

    // Title text
    gui_draw_text(win->x + 5, win->y + 5, win->title, 0x000000, 0xC0C0C0);

    // Content area
    gui_draw_rect(win->x + 2, win->y + 24, win->width - 4, win->height - 26, 0xFFFFFF);
}

void window_handle_event(window_t *win, int event_type, int x, int y) {
    (void)win; (void)event_type; (void)x; (void)y;
}
