#include "gui.h"
#include "../arch/x86/framebuffer.h"

int gui_point_in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && py >= y && px < x + w && py < y + h;
}

void gui_draw_rect(int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    int start_y = y < 0 ? 0 : y;
    int end_y = (y + h) > (int)fb_height ? (int)fb_height : (y + h);
    int start_x = x < 0 ? 0 : x;
    int end_x = (x + w) > (int)fb_width ? (int)fb_width : (x + w);
    if (start_y >= end_y || start_x >= end_x) return;
    for (int yy = start_y; yy < end_y; yy++) {
        for (int xx = start_x; xx < end_x; xx++) {
            fb_putpixel(xx, yy, color);
        }
    }
}

void gui_draw_text(int x, int y, const char *text, uint32_t fg, uint32_t bg) {
    int cx = x;
    /* quick vertical bounds check: glyphs are 8px tall */
    if (y + 8 < 0 || y >= (int)fb_height) return;
    while (*text) {
        if (cx >= (int)fb_width) break; /* no more room on the right */
        fb_putchar(cx, y, *text, fg, bg);
        cx += 8; // assume 8px glyph width
        text++;
    }
}
