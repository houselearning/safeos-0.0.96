// calculator.c
#include "calculator.h"
#include "../window.h"
#include "../gui.h"
#include "../cursor.h"

static window_t *win = NULL;
static char display[32] = "0";
static char expr[32] = "";
static int expr_len = 0;

typedef struct {
    int x, y, w, h;
    char label;
} button_t;

static button_t buttons[] = {
    {10, 50, 50, 50, '7'}, {70, 50, 50, 50, '8'}, {130, 50, 50, 50, '9'}, {190, 50, 50, 50, '/'},
    {10, 110, 50, 50, '4'}, {70, 110, 50, 50, '5'}, {130, 110, 50, 50, '6'}, {190, 110, 50, 50, '*'},
    {10, 170, 50, 50, '1'}, {70, 170, 50, 50, '2'}, {130, 170, 50, 50, '3'}, {190, 170, 50, 50, '-'},
    {10, 230, 50, 50, '0'}, {70, 230, 50, 50, 'C'}, {130, 230, 50, 50, '='}, {190, 230, 50, 50, '+'},
};

#define NUM_BUTTONS (sizeof(buttons)/sizeof(button_t))

void calculator_open(void) {
    if (win) return;
    win = window_create("Calculator", 120, 120, 300, 400);
    strcpy(display, "0");
    expr[0] = '\0';
    expr_len = 0;
}

void calculator_handle_event(gui_event_t *ev) {
    if (!win) return;
    if (ev->type == MOUSE_DOWN && ev->button == 1) {
        int mx = cursor_get_x() - win->x;
        int my = cursor_get_y() - win->y;
        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (mx >= buttons[i].x && mx < buttons[i].x + buttons[i].w &&
                my >= buttons[i].y && my < buttons[i].y + buttons[i].h) {
                char c = buttons[i].label;
                if (c == 'C') {
                    strcpy(display, "0");
                    expr[0] = '\0';
                    expr_len = 0;
                } else if (c == '=') {
                    // simple eval, assume + - * /
                    int result = 0;
                    sscanf(expr, "%d", &result); // stub
                    sprintf(display, "%d", result);
                    expr[0] = '\0';
                    expr_len = 0;
                } else {
                    if (expr_len < sizeof(expr)-1) {
                        expr[expr_len++] = c;
                        expr[expr_len] = '\0';
                        strcpy(display, expr);
                    }
                }
                break;
            }
        }
    }
}

void calculator_draw(void) {
    if (!win) return;
    window_draw(win);
    // draw display
    gui_draw_rect(win->x + 10, win->y + 30, 240, 20, 0x000000);
    gui_draw_text(win->x + 15, win->y + 32, display, 0xFFFFFF, 0x000000);
    // draw buttons
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gui_draw_rect(win->x + buttons[i].x, win->y + buttons[i].y, buttons[i].w, buttons[i].h, 0xC0C0C0);
        char buf[2] = {buttons[i].label, '\0'};
        gui_draw_text(win->x + buttons[i].x + 20, win->y + buttons[i].y + 15, buf, 0x000000, 0xC0C0C0);
    }
}
