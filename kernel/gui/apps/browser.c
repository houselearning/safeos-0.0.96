#include "browser.h"
#include "../window.h"
#include "../../core/net.h"
#include "../gui.h"
#include "../../core/string.h"

static window_t *win;

static char url[256] = "http://sbext.houselearning.org/";
static char page[8192];

static int url_cursor = 0;
static int url_focused = 0;

void browser_open(void) {
    win = window_create("Browser", 200, 100, 800, 600);
    url_cursor = strlen(url);
    page[0] = 0;
}

void browser_handle_event(gui_event_t *ev) {
    if (!win) return;

    int url_x = win->x + 10;
    int url_y = win->y + 30;

    /* CLICK URL BAR */
    if (ev->type == GUI_EVENT_MOUSE_DOWN) {
        if (gui_point_in_rect(ev->x, ev->y, url_x, url_y, 780, 24)) {
            url_focused = 1;
        } else {
            url_focused = 0;
        }
    }

    if (!url_focused)
        return;

    /* TEXT INPUT */
    if (ev->type == GUI_EVENT_KEY_CHAR) {
        if (url_cursor < (int)sizeof(url) - 1) {
            url[url_cursor++] = ev->ch;
            url[url_cursor] = 0;
        }
    }

    /* SPECIAL KEYS */
    if (ev->type == GUI_EVENT_KEY_DOWN) {

        /* BACKSPACE */
        if (ev->key == GUI_KEY_BACKSPACE && url_cursor > 0) {
            url[--url_cursor] = 0;
        }

        /* ENTER → FETCH PAGE */
        if (ev->key == GUI_KEY_ENTER) {
            memset(page, 0, sizeof(page));
            net_http_get(url, page, sizeof(page));
        }
    }
}

void browser_draw(void) {
    if (!win) return;

    window_begin_draw(win);

    int url_x = win->x + 10;
    int url_y = win->y + 30;
    int page_x = win->x + 10;
    int page_y = win->y + 70;

    /* URL BAR */
    gui_draw_rect(
        url_x,
        url_y,
        780,
        24,
        url_focused ? GUI_COLOR_HIGHLIGHT : GUI_COLOR_MENU
    );

    gui_draw_text(url_x + 6, url_y + 6, url, GUI_COLOR_TEXT);

    /* PAGE CONTENT (RAW TEXT) */
    gui_draw_rect(page_x, page_y, 780, 500, GUI_COLOR_BACKGROUND);
    gui_draw_text(page_x + 6, page_y + 6, page, GUI_COLOR_TEXT);

    window_end_draw(win);
}