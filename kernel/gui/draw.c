#include "gui.h"
#include "../arch/x86/framebuffer.h"

int gui_point_in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && py >= y && px < x + w && py < y + h;
}

void gui_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            fb_putpixel(xx, yy, color);
        }
    }
}

void gui_draw_text(int x, int y, const char *text, uint32_t color) {
    (void)color;
    int cx = x;
    while (*text) {
        fb_putchar(cx, y, *text, color, 0);
        cx += 8; // assume 8px glyph width
        text++;
    }
}
