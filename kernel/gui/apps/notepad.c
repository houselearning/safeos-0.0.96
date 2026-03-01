// notepad.c
#include "notepad.h"
#include "../window.h"
#include "../../core/fs.h"

static window_t *win;
static char buffer[4096];
static int len = 0;

void notepad_open(const char *path) {
    win = window_create("Notepad", 80, 80, 500, 400);
    len = fs_read_file(path, buffer, sizeof(buffer)-1);
    if (len < 0) len = 0;
    buffer[len] = '\0';
}

void notepad_handle_event(gui_event_t *ev) {
    if (!win) return;
    // handle keyboard input, backspace, etc.
}

void notepad_draw(void) {
    if (!win) return;
    window_begin_draw(win);
    // draw text buffer
    window_end_draw(win);
}
