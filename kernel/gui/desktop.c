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

typedef struct {
    int x, y, w, h;
    const char *name;
    void (*open_func)(const char *);
} icon_t;

static icon_t icons[] = {
    {50, 50, 64, 64, "Notepad", notepad_open},
    {150, 50, 64, 64, "Calculator", calculator_open},
    {250, 50, 64, 64, "Spreadsheet", spreadsheet_open},
    {350, 50, 64, 64, "Files", file_explorer_open},
    {450, 50, 64, 64, "Browser", browser_open},
};

#define NUM_ICONS (sizeof(icons)/sizeof(icon_t))

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
        for (int i = 0; i < (int)NUM_ICONS; i++) {
            if (mx >= icons[i].x && mx < icons[i].x + icons[i].w &&
                my >= icons[i].y && my < icons[i].y + icons[i].h) {
                icons[i].open_func(NULL);
                break;
            }
        }
    } else if (ev->type == KEY_CHAR) {
        // Send to notepad if open
        extern void notepad_handle_event(gui_event_t *ev);
        notepad_handle_event(ev);
    } else if (ev->type == MOUSE_DOWN || ev->type == MOUSE_UP || ev->type == MOUSE_MOVE) {
        // Send to calculator if click in window
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
    framebuffer_clear(0x202020);
    // draw icons
    for (int i = 0; i < (int)NUM_ICONS; i++) {
        gui_draw_rect(icons[i].x, icons[i].y, icons[i].w, icons[i].h, 0x808080);
        gui_draw_text(icons[i].x + 5, icons[i].y + 20, icons[i].name, 0xFFFFFF, 0x808080);
    }
    // draw apps
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
