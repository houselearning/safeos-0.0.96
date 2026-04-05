// calculator.c
// Force recompile
#include "calculator.h"
#include "../window.h"
#include "../gui.h"
#include "../cursor.h"
#include <stddef.h>
#include <./core/string.h>
#include <./core/stdio.h>

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

void calculator_open(const char *path) {
    (void)path;
    if (win) {
        if (window_is_minimized(win) || !window_is_visible(win)) {
            window_restore(win);
            return;
        }
        return;
    }
    win = window_create("Calculator", 120, 120, 300, 400);
    strcpy(display, "0");
    expr[0] = '\0';
    expr_len = 0;
}

void calculator_handle_event(gui_event_t *ev) {
    if (!win || window_is_minimized(win) || !window_is_visible(win)) return;
    if (ev->type == MOUSE_DOWN && ev->button == 1) {
        int mx = cursor_get_x() - win->x;
        int my = cursor_get_y() - win->y;
        for (int i = 0; i < (int)NUM_BUTTONS; i++) {
            if (mx >= buttons[i].x && mx < buttons[i].x + buttons[i].w &&
                my >= buttons[i].y && my < buttons[i].y + buttons[i].h) {
                char c = buttons[i].label;
                if (c == 'C') {
                    strcpy(display, "0");
                    expr[0] = '\0';
                    expr_len = 0;
                } else if (c == '=') {
                    // Evaluate arithmetic expression supporting + - * /
                    int ok = 0;
                    int result = 0;
                    // Simple expression evaluator: tokenize integers and ops
                    const char *p = expr;
                    int values[32]; int vtop = 0;

                    while (*p == ' ') p++;
                    // Parse first number
                    int sign = 1;
                    if (*p == '+') { p++; }
                    else if (*p == '-') { sign = -1; p++; }
                    if (*p < '0' || *p > '9') { ok = 0; goto finish_eval; }
                    int cur = 0;
                    while (*p >= '0' && *p <= '9') { cur = cur * 10 + (*p - '0'); p++; }
                    values[vtop++] = cur * sign;
                    ok = 1;

                    while (*p) {
                        while (*p == ' ') p++;
                        char op = *p++;
                        while (*p == ' ') p++;
                        if (op != '+' && op != '-' && op != '*' && op != '/') { ok = 0; break; }
                        // parse next number
                        int s = 1;
                        if (*p == '+') { p++; }
                        else if (*p == '-') { s = -1; p++; }
                        if (*p < '0' || *p > '9') { ok = 0; break; }
                        int n = 0;
                        while (*p >= '0' && *p <= '9') { n = n * 10 + (*p - '0'); p++; }
                        n *= s;

                        // handle precedence: if op is * or /, apply immediately
                        if (op == '*') {
                            if (vtop == 0) { ok = 0; break; }
                            values[vtop-1] = values[vtop-1] * n;
                        } else if (op == '/') {
                            if (vtop == 0 || n == 0) { ok = 0; break; }
                            values[vtop-1] = values[vtop-1] / n;
                        } else {
                            // + or - push
                            if (op == '+') values[vtop++] = n;
                            else values[vtop++] = -n;
                        }
                    }

                    finish_eval:
                    if (ok) {
                        int acc = 0;
                        for (int i = 0; i < vtop; ++i) acc += values[i];
                        result = acc;
                        sprintf(display, "%d", result);
                    } else {
                        strcpy(display, "ERR");
                    }
                    expr[0] = '\0';
                    expr_len = 0;
                } else {
                    if (expr_len < (int)(sizeof(expr)-1)) {
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
    for (int i = 0; i < (int)NUM_BUTTONS; i++) {
        gui_draw_rect(win->x + buttons[i].x, win->y + buttons[i].y, buttons[i].w, buttons[i].h, 0xC0C0C0);
        char buf[2] = {buttons[i].label, '\0'};
        gui_draw_text(win->x + buttons[i].x + 20, win->y + buttons[i].y + 15, buf, 0x000000, 0xC0C0C0);
    }
}
