// notepad.c
#include "notepad.h"
#include "../window.h"
#include "../gui.h"
#include "../../core/fs.h"
#include <stddef.h>

static window_t *win = NULL;
static char buffer[4096];
static int len = 0;

void notepad_open(const char *path) {
    if (win) return; // already open
    win = window_create("Notepad", 80, 80, 500, 400);
    if (path) {
        len = fs_read_file(path, buffer, sizeof(buffer)-1);
        if (len < 0) len = 0;
    } else {
        len = 0;
    }
    buffer[len] = '\0';
}

void notepad_handle_event(gui_event_t *ev) {
    if (!win) return;
    if (ev->type == KEY_CHAR) {
        if (ev->ch == 8) { // backspace
            if (len > 0) len--;
        } else if (ev->ch >= 32 && len < (int)(sizeof(buffer)-1)) {
            buffer[len++] = ev->ch;
        }
        buffer[len] = '\0';
    }
}

void notepad_draw(void) {
    if (!win) return;
    window_draw(win);
    // draw text
    int x = win->x + 5;
    int y = win->y + 30;
    char *p = buffer;
    while (*p) {
        if (*p == '\n') {
            y += 12;
            x = win->x + 5;
        } else {
            gui_draw_text(x, y, p, 0x000000, 0xFFFFFF);
            x += 8;
            if (x > win->x + win->width - 10) {
                y += 12;
                x = win->x + 5;
            }
        }
        p++;
    }
}
