// spreadsheet.c
#include "spreadsheet.h"
#include "../window.h"
#include "../gui.h"
#include "../cursor.h"
#include <stddef.h>
#include <./core/string.h>
#include <./core/stdio.h>

#define ROWS 16
#define COLS 8

static window_t *win = NULL;
static char cells[ROWS][COLS][32];
static int sel_row = 0, sel_col = 0;

void spreadsheet_open(const char *path) {
    (void)path;
    if (win) return;
    win = window_create("Spreadsheet", 150, 150, 700, 500);
    memset(cells, 0, sizeof(cells));
}

void spreadsheet_handle_event(gui_event_t *ev) {
    if (!win) return;
    if (ev->type == MOUSE_DOWN && ev->button == 1) {
        int mx = cursor_get_x() - win->x;
        int my = cursor_get_y() - win->y;
        int cell_w = (win->width - 40) / COLS;
        int cell_h = (win->height - 40) / ROWS;
        if (mx >= 40 && my >= 20) {
            int c = (mx - 40) / cell_w;
            int r = (my - 20) / cell_h;
            if (c < COLS && r < ROWS) {
                sel_col = c;
                sel_row = r;
            }
        }
    } else if (ev->type == KEY_CHAR) {
        if (ev->ch == 8) { // backspace
            int len = strlen(cells[sel_row][sel_col]);
            if (len > 0) cells[sel_row][sel_col][len-1] = '\0';
        } else if (ev->ch >= 32) {
            int len = strlen(cells[sel_row][sel_col]);
            if (len < 31) {
                cells[sel_row][sel_col][len] = ev->ch;
                cells[sel_row][sel_col][len+1] = '\0';
            }
        }
    }
}

void spreadsheet_draw(void) {
    if (!win) return;
    window_draw(win);
    int cell_w = (win->width - 40) / COLS;
    int cell_h = (win->height - 40) / ROWS;
    // draw headers
    for (int c = 0; c < COLS; c++) {
        char buf[4];
        sprintf(buf, "%c", 'A' + c);
        gui_draw_text(win->x + 40 + c * cell_w + 5, win->y + 25, buf, 0x000000, 0xFFFFFF);
    }
    for (int r = 0; r < ROWS; r++) {
        char buf[4];
        sprintf(buf, "%d", r+1);
        gui_draw_text(win->x + 5, win->y + 20 + (r+1) * cell_h + 5, buf, 0x000000, 0xFFFFFF);
    }
    // draw grid
    for (int r = 0; r <= ROWS; r++) {
        int y = win->y + 20 + r * cell_h;
        gui_draw_rect(win->x + 40, y, win->width - 40, 1, 0x000000);
    }
    for (int c = 0; c <= COLS; c++) {
        int x = win->x + 40 + c * cell_w;
        gui_draw_rect(x, win->y + 20, 1, win->height - 20, 0x000000);
    }
    // draw cells
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int x = win->x + 40 + c * cell_w + 2;
            int y = win->y + 20 + (r+1) * cell_h - cell_h + 2;
            uint32_t bg = (r == sel_row && c == sel_col) ? 0xFFFF00 : 0xFFFFFF;
            gui_draw_text(x, y, cells[r][c], 0x000000, bg);
        }
    }
}
