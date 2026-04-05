#include "window.h"
#include "gui.h"
#include <stddef.h>

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
            windows[i].minimized = 0;
            windows[i].title = title;
            return &windows[i];
        }
    }
    return NULL;
}

void window_begin_draw(window_t *win) {
    (void)win;
}

void window_end_draw(window_t *win) {
    (void)win;
}

int window_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_WINDOWS; ++i) {
        if (window_used[i] && (windows[i].visible || windows[i].minimized)) {
            count++;
        }
    }
    return count;
}

window_t* window_get(int idx) {
    int found = 0;
    for (int i = 0; i < MAX_WINDOWS; ++i) {
        if (window_used[i] && (windows[i].visible || windows[i].minimized)) {
            if (found == idx) return &windows[i];
            found++;
        }
    }
    return NULL;
}

void window_close(window_t *win) {
    if (!win) return;
    win->visible = 0;
    win->minimized = 0;
}

void window_minimize(window_t *win) {
    if (!win) return;
    if (!win->visible) return;
    win->minimized = 1;
    win->visible = 0;
}

void window_restore(window_t *win) {
    if (!win) return;
    win->visible = 1;
    win->minimized = 0;
}

int window_is_minimized(window_t *win) {
    return win && win->minimized;
}

int window_is_visible(window_t *win) {
    return win && win->visible && !win->minimized;
}

int window_handle_title_bar_click(int x, int y) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!window_used[i]) continue;
        window_t *win = &windows[i];
        if (!win->visible) continue;

        int title_x1 = win->x + 2;
        int title_y1 = win->y + 2;
        int title_w = win->width - 4;
        int title_h = 24;

        if (!gui_point_in_rect(x, y, title_x1, title_y1, title_w, title_h)) {
            continue;
        }

        int close_x = win->x + win->width - 26;
        int close_y = win->y + 6;
        int min_x = win->x + win->width - 50;
        int min_y = win->y + 6;

        if (gui_point_in_rect(x, y, close_x, close_y, 16, 12)) {
            window_close(win);
            return 1;
        }
        if (gui_point_in_rect(x, y, min_x, min_y, 16, 12)) {
            window_minimize(win);
            return 1;
        }

        int last_used = -1;
        for (int j = 0; j < MAX_WINDOWS; j++) {
            if (window_used[j]) last_used = j;
        }
        if (last_used >= 0 && i != last_used) {
            window_t tmp = windows[i];
            windows[i] = windows[last_used];
            windows[last_used] = tmp;
            window_used[i] = window_used[last_used];
            window_used[last_used] = 1;
        }

        return 1;
    }
    return 0;
}

void window_draw(window_t *win) {
    if (!win || !win->visible || win->minimized) return;

    /* Window frame and shadow for a polished desktop look. */
    gui_draw_rect(win->x + 2, win->y + 2, win->width, win->height, 0x000000);
    gui_draw_rect(win->x, win->y, win->width, win->height, 0x232B36);
    gui_draw_rect(win->x + 1, win->y + 1, win->width - 2, win->height - 2, 0xFFFFFF);

    /* Title bar */
    gui_draw_rect(win->x + 2, win->y + 2, win->width - 4, 26, 0x315A9F);
    gui_draw_rect(win->x + 2, win->y + 2, 10, 10, 0x7AA3F1);
    gui_draw_rect(win->x + 16, win->y + 6, 6, 6, 0x7AA3F1);
    gui_draw_rect(win->x + 26, win->y + 6, 6, 6, 0x7AA3F1);

    gui_draw_text(win->x + 36, win->y + 6, win->title, 0xFFFFFF, 0x315A9F);

    int min_x = win->x + win->width - 52;
    int close_x = win->x + win->width - 28;
    int button_y = win->y + 6;
    gui_draw_rect(min_x, button_y, 18, 14, 0xF0D24C);
    gui_draw_rect(close_x, button_y, 18, 14, 0xF06B6B);
    gui_draw_text(min_x + 5, button_y + 2, "_", 0x000000, 0xF0D24C);
    gui_draw_text(close_x + 5, button_y + 2, "X", 0x000000, 0xF06B6B);

    gui_draw_rect(win->x + 3, win->y + 28, win->width - 6, win->height - 31, 0xECECEC);
}

void window_handle_event(window_t *win, int event_type, int x, int y) {
    (void)win;
    (void)event_type;
    (void)x;
    (void)y;
}
