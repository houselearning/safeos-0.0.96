#include "desktop.h"
#include "arch/x86/framebuffer.h"
#include "window.h"
#include "apps/notepad.h"
#include "apps/calculator.h"
#include "apps/spreadsheet.h"
#include "apps/file_explorer.h"
#include "apps/browser.h"

static int show_startup = 1;

void desktop_show_startup_screen(const char *title, const char *msg) {
    framebuffer_clear(0x000000);
    // draw title + message + fake progress bar
    // (you’d implement text drawing and rectangles here)
}

void desktop_init_home(void) {
    show_startup = 0;
    // create icon structures for each app
    // e.g., icon positions and click handlers
}

void desktop_handle_event(gui_event_t *ev) {
    if (show_startup) return;

    // handle mouse clicks on icons, windows, context menus
    // right-click: open context menu
}

void desktop_draw(void) {
    if (show_startup) return;
    framebuffer_clear(0x202020);
    // draw icons, open windows, context menus
}
