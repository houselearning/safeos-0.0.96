// spreadsheet.c
#include "spreadsheet.h"
#include "window.h"

#define ROWS 16
#define COLS 8

static window_t *win;
static char cells[ROWS][COLS][32];

void spreadsheet_open(void) {
    win = window_create("Spreadsheet", 150, 150, 700, 500);
}

void spreadsheet_handle_event(gui_event_t *ev) {
    // click to select cell, keyboard to edit
    // optionally parse formulas starting with '='
}

void spreadsheet_draw(void) {
    if (!win) return;
    window_begin_draw(win);
    // draw grid and cell contents
    window_end_draw(win);
}
