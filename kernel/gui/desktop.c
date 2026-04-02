#include "desktop.h"
#include "../arch/x86/framebuffer.h"
#include "window.h"
#include "cursor.h"
#include "apps/notepad.h"
#include "apps/calculator.h"
#include "apps/spreadsheet.h"
#include "apps/file_explorer.h"
#include "apps/browser.h"
#include "gui.h"
#include <stddef.h>

static int show_startup = 1;

/* Desktop layout constants matching the provided mockup */
static const int APP_ICON_X = 20;
static const int APP_ICON_Y = 20;
static const int APP_ICON_W = 80;
static const int APP_ICON_H = 80;

/* Taskbar constants */
static const int TASKBAR_H = 42;

void desktop_show_startup_screen(const char *title, const char *msg) {
    framebuffer_clear(0x000000);
    gui_draw_text(100, 100, title, 0xFFFFFF, 0x000000);
    gui_draw_text(100, 120, msg, 0xFFFFFF, 0x000000);
    // fake progress
    gui_draw_rect(100, 140, 200, 10, 0xFFFFFF);
    gui_draw_rect(102, 142, 196, 6, 0x0000FF);
}

void desktop_init_home(void) {
    show_startup = 0;
}

void desktop_handle_event(gui_event_t *ev) {
    if (show_startup) return;

    if (ev->type == MOUSE_DOWN && ev->button == 1) { // Left click
        int mx = cursor_get_x();
        int my = cursor_get_y();
        /* Top-left app icon */
        if (mx >= APP_ICON_X && mx < APP_ICON_X + APP_ICON_W &&
            my >= APP_ICON_Y && my < APP_ICON_Y + APP_ICON_H) {
            notepad_open(NULL);
            return;
        }

        /* Taskbar small icon (left) */
        int tb_icon_x = 8;
        int tb_icon_y = fb_height - TASKBAR_H + 6;
        int tb_icon_w = 32;
        int tb_icon_h = 32;
        if (mx >= tb_icon_x && mx < tb_icon_x + tb_icon_w &&
            my >= tb_icon_y && my < tb_icon_y + tb_icon_h) {
            file_explorer_open(NULL);
            return;
        }

        /* Start button (approximate circle hit test) */
        int start_cx = fb_width / 2;
        int start_cy = fb_height - TASKBAR_H / 2;
        int start_r = 22;
        int dx = mx - start_cx;
        int dy = my - start_cy;
        if (dx * dx + dy * dy <= start_r * start_r) {
            browser_open(NULL);
            return;
        }
    } else if (ev->type == KEY_CHAR) {
        extern void notepad_handle_event(gui_event_t *ev);
        notepad_handle_event(ev);
    }

    /* Forward other mouse/key events to open app windows */
    if (ev->type == MOUSE_DOWN || ev->type == MOUSE_UP || ev->type == MOUSE_MOVE) {
        extern void calculator_handle_event(gui_event_t *ev);
        calculator_handle_event(ev);
        extern void spreadsheet_handle_event(gui_event_t *ev);
        spreadsheet_handle_event(ev);
        extern void file_explorer_handle_event(gui_event_t *ev);
        file_explorer_handle_event(ev);
        extern void browser_handle_event(gui_event_t *ev);
        browser_handle_event(ev);
    }
}

void desktop_draw(void) {
    if (show_startup) return;

    /* White background (colored-in) */
    framebuffer_clear(0xFFFFFF);

    /* Top-left app icon (outlined) */
    gui_draw_rect(APP_ICON_X, APP_ICON_Y, APP_ICON_W, APP_ICON_H, 0xFFFFFF);
    gui_draw_rect(APP_ICON_X, APP_ICON_Y, APP_ICON_W, 1, 0x000000);
    gui_draw_rect(APP_ICON_X, APP_ICON_Y, 1, APP_ICON_H, 0x000000);
    gui_draw_rect(APP_ICON_X + APP_ICON_W - 1, APP_ICON_Y, 1, APP_ICON_H, 0x000000);
    gui_draw_rect(APP_ICON_X, APP_ICON_Y + APP_ICON_H - 1, APP_ICON_W, 1, 0x000000);
    gui_draw_text(APP_ICON_X + 8, APP_ICON_Y + (APP_ICON_H / 2) - 4, "app icon", 0x000000, 0xFFFFFF);
    gui_draw_text(APP_ICON_X, APP_ICON_Y + APP_ICON_H + 10, "{app label}", 0x000000, 0xFFFFFF);

    /* Bottom taskbar */
    gui_draw_rect(0, fb_height - TASKBAR_H, fb_width, TASKBAR_H, 0x202020);

    /* Left: small image icon on taskbar */
    int tb_icon_x = 8;
    int tb_icon_y = fb_height - TASKBAR_H + 6;
    int tb_icon_w = 32;
    int tb_icon_h = 32;
    gui_draw_rect(tb_icon_x, tb_icon_y, tb_icon_w, tb_icon_h, 0xFFFFFF);
    gui_draw_rect(tb_icon_x, tb_icon_y, tb_icon_w, 1, 0x000000);
    gui_draw_rect(tb_icon_x, tb_icon_y, 1, tb_icon_h, 0x000000);
    gui_draw_rect(tb_icon_x + tb_icon_w - 1, tb_icon_y, 1, tb_icon_h, 0x000000);
    gui_draw_text(tb_icon_x + tb_icon_w + 8, tb_icon_y + 8, "_  app on taskbar. when clicked, open app.", 0xFFFFFF, 0x202020);

    /* Center: circular Start button */
    int start_cx = fb_width / 2;
    int start_cy = fb_height - TASKBAR_H / 2;
    int start_r = 22;
    gui_draw_circle(start_cx + 2, start_cy + 2, start_r + 2, 0x404040);
    gui_draw_circle(start_cx, start_cy, start_r, 0xC0C0C0);
    const char *start_label = "start button";
    int label_len = 0; const char *p = start_label; while (*p) { label_len++; p++; }
    int label_x = start_cx - (label_len * 4);
    int label_y = start_cy - 4;
    gui_draw_text(label_x, label_y, start_label, 0x000000, 0xC0C0C0);

    /* Right: time / volume / wifi / settings */
    int right_x = fb_width - 220;
    int right_y = fb_height - TASKBAR_H + 6;
    gui_draw_text(right_x, right_y, "{TIME} (AM/PM)", 0xFFFFFF, 0x202020);
    gui_draw_text(right_x, right_y + 12, "Volume: (audio lvl)", 0xFFFFFF, 0x202020);
    gui_draw_text(right_x, right_y + 24, "Wifi: (wifi)", 0xFFFFFF, 0x202020);
    gui_draw_text(right_x, right_y + 36, "Settings", 0xFFFFFF, 0x202020);

    /* Draw any open apps/windows */
    extern void notepad_draw(void);
    notepad_draw();
    extern void calculator_draw(void);
    calculator_draw();
    extern void spreadsheet_draw(void);
    spreadsheet_draw();
    extern void file_explorer_draw(void);
    file_explorer_draw();
    extern void browser_draw(void);
    browser_draw();
}
