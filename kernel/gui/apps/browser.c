#include "browser.h"
#include "window.h"
#include "net.h"

static window_t *win;
static char url[256] = "http://example.com/";
static char page[8192];

void browser_open(void) {
    win = window_create("Browser", 200, 100, 800, 600);
}

void browser_handle_event(gui_event_t *ev) {
    // handle typing into URL bar, pressing Enter to fetch
    // net_http_get(url, page, sizeof(page));
}

void browser_draw(void) {
    if (!win) return;
    window_begin_draw(win);
    // draw URL bar and page text
    window_end_draw(win);
}
