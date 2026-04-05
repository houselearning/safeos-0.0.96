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
#include "../core/stdio.h"

static int show_startup = 1;
static const int TASKBAR_H = 50;

static uint32_t desktop_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static void desktop_draw_wallpaper(void) {
    /* Soft, light panels for depth and polish */
    int panel_w = fb_width / 3;
    int panel_h = fb_height / 7;
    for (int i = 0; i < 3; ++i) {
        int x = 48 + i * (panel_w + 16);
        int y = fb_height / 6 + (i % 2) * 24;
        gui_draw_rect(x, y, panel_w, panel_h, desktop_rgb(0x3A, 0x60, 0xA0));
        gui_draw_rect(x + 8, y + 8, panel_w - 16, panel_h - 16, desktop_rgb(0x55, 0x7C, 0xC8));
    }
}

static void desktop_draw_icon(int x, int y, const char *label, uint32_t color) {
    gui_draw_rect(x, y, 72, 72, 0x2F3F6C);
    gui_draw_rect(x + 2, y + 2, 68, 68, color);
    gui_draw_rect(x + 6, y + 6, 60, 18, color);
    gui_draw_rect(x + 10, y + 28, 52, 28, 0xFFFFFF);
    gui_draw_rect(x + 18, y + 36, 36, 12, color);
    gui_draw_rect(x - 2, y + 74, 76, 16, 0x182139);
    gui_draw_text(x + 6, y + 76, label, 0xFFFFFF, 0x182139);
}

static void desktop_draw_start_button(int taskbar_top) {
    int button_x = 14;
    int button_y = taskbar_top + 10;
    gui_draw_rect(button_x, button_y, 100, 30, 0x2A6EFE);
    gui_draw_rect(button_x + 2, button_y + 2, 96, 26, 0x4B8CFF);
    gui_draw_text(button_x + 14, button_y + 8, "Start", 0xFFFFFF, 0x4B8CFF);
}

static void desktop_draw_system_tray(int taskbar_top) {
    int tray_x = fb_width - 224;
    int tray_y = taskbar_top + 10;
    gui_draw_rect(tray_x, tray_y, 214, 30, 0x232A34);
    gui_draw_rect(tray_x + 8, tray_y + 6, 16, 16, 0x4B88FF);
    gui_draw_rect(tray_x + 28, tray_y + 6, 16, 16, 0x5DD25C);
    gui_draw_rect(tray_x + 48, tray_y + 6, 16, 16, 0xF2C94C);
    gui_draw_text(tray_x + 74, tray_y + 8, "11:42", 0xFFFFFF, 0x232A34);
    gui_draw_text(tray_x + 140, tray_y + 8, "WiFi", 0xFFFFFF, 0x232A34);
    gui_draw_text(tray_x + 140, tray_y + 18, "Vol 44%", 0xBBBBBB, 0x232A34);
}

void desktop_show_startup_screen(const char *title, const char *msg) {
    framebuffer_clear(0x000000);
    gui_draw_text(100, 100, title, 0xFFFFFF, 0x000000);
    gui_draw_text(100, 120, msg, 0xFFFFFF, 0x000000);
    gui_draw_rect(100, 140, 200, 10, 0xFFFFFF);
    gui_draw_rect(102, 142, 196, 6, 0x0000FF);
}

void desktop_init_home(void) {
    show_startup = 0;
}

void desktop_handle_event(gui_event_t *ev) {
    if (show_startup) return;

    if (ev->type == MOUSE_DOWN) {
        if (window_handle_title_bar_click(ev->x, ev->y)) {
            return;
        }
    }

    if (ev->type == MOUSE_DOWN && ev->button == 1) {
        static const struct {
            const char *name;
            const char *label;
            uint32_t color;
            void (*fn)(const char *);
        } apps[] = {
            {"Notepad", "Note", 0x64A0FF, notepad_open},
            {"Calculator", "Calc", 0xFFD23E, calculator_open},
            {"Spreadsheet", "Sheet", 0x7ED27A, spreadsheet_open},
            {"Explorer", "Files", 0xF78F3D, file_explorer_open},
            {"Browser", "Web", 0xA45BFF, browser_open},
        };

        int icon_x = 24;
        int icon_y = 24;
        for (int i = 0; i < (int)(sizeof(apps) / sizeof(apps[0])); ++i) {
            if (gui_point_in_rect(ev->x, ev->y, icon_x, icon_y, 72, 72)) {
                apps[i].fn(NULL);
                return;
            }
            icon_y += 106;
        }

        int taskbar_top = fb_height - TASKBAR_H;
        int slot_x = 84;
        for (int i = 0; i < window_count(); ++i) {
            window_t *win = window_get(i);
            if (!win) continue;
            int w = 88;
            int h = 26;
            int y = taskbar_top + 12;
            if (window_is_minimized(win) && gui_point_in_rect(ev->x, ev->y, slot_x, y, w, h)) {
                window_restore(win);
                return;
            }
            slot_x += w + 8;
        }
    }

    if (ev->type == KEY_CHAR) {
        notepad_handle_event(ev);
        calculator_handle_event(ev);
        spreadsheet_handle_event(ev);
        file_explorer_handle_event(ev);
        browser_handle_event(ev);
    }

    if (ev->type == MOUSE_DOWN || ev->type == MOUSE_UP || ev->type == MOUSE_MOVE) {
        calculator_handle_event(ev);
        spreadsheet_handle_event(ev);
        file_explorer_handle_event(ev);
        browser_handle_event(ev);
        notepad_handle_event(ev);
    }
}

void desktop_draw(void) {
    if (show_startup) return;
    
    if (!fb_address || fb_width == 0 || fb_height == 0) {
        return;
    }

    extern uint32_t *backbuffer;

    desktop_draw_wallpaper();

    static const struct {
        const char *name;
        const char *label;
        uint32_t color;
    } icons[] = {
        {"Notepad", "Note", 0x64A0FF},
        {"Calculator", "Calc", 0xFFD23E},
        {"Spreadsheet", "Sheet", 0x7ED27A},
        {"Explorer", "Files", 0xF78F3D},
        {"Browser", "Web", 0xA45BFF},
    };

    int icon_x = 32;
    int icon_y = 28;
    for (int i = 0; i < (int)(sizeof(icons) / sizeof(icons[0])); ++i) {
        desktop_draw_icon(icon_x, icon_y, icons[i].label, icons[i].color);
        icon_y += 106;
    }

    int taskbar_top = fb_height - TASKBAR_H;
    gui_draw_rect(0, taskbar_top, fb_width, TASKBAR_H, 0x1D2027);
    gui_draw_rect(0, taskbar_top, fb_width, 2, 0x4B4F58);

    desktop_draw_start_button(taskbar_top);

    int tb_x = 84;
    for (int i = 0; i < window_count(); ++i) {
        window_t *win = window_get(i);
        if (!win) continue;
        int w = 100;
        int y = taskbar_top + 12;
        uint32_t bg = window_is_minimized(win) ? 0x32363D : 0x40454E;
        gui_draw_rect(tb_x, y, w, 26, bg);
        gui_draw_text(tb_x + 6, y + 9, win->title, 0xFFFFFF, bg);
        tb_x += w + 10;
    }

    desktop_draw_system_tray(taskbar_top);

    char status[96];
    sprintf(status, "FB %ux%u @ %ubpp", fb_width, fb_height, fb_bpp);
    gui_draw_text(16, fb_height - TASKBAR_H - 18, status, 0xE0E0E0, 0x1D2027);
}

