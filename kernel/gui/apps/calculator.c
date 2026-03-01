// calculator.c
#include "calculator.h"
#include "window.h"
#include "gui.h"

static window_t *win;
static char display[32] = "0";

void calculator_open(void) {
    win = window_create("Calculator", 120, 120, 300, 400);
}

void calculator_handle_event(gui_event_t *ev) {
    if (!win) return;
    if (ev->type == GUI_EVENT_MOUSE_DOWN && ev->button == GUI_MOUSE_LEFT) {
        // detect which button (0-9, +, -, *, /, =, C) was clicked
        // update internal state and display
    }
}

void calculator_draw(void) {
    if (!win) return;
    window_begin_draw(win);
    // draw display and buttons grid
    window_end_draw(win);
}
