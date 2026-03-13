#include "cursor.h"
#include "../arch/x86/framebuffer.h"

int cursor_x = 100;
int cursor_y = 100;

void cursor_init(void) {
    cursor_x = fb_width / 2;
    cursor_y = fb_height / 2;
}

void cursor_move(int dx, int dy) {
    cursor_x += dx;
    cursor_y += dy;

    if (cursor_x < 0) cursor_x = 0;
    if (cursor_y < 0) cursor_y = 0;
    if (cursor_x >= (int)fb_width) cursor_x = fb_width - 1;
    if (cursor_y >= (int)fb_height) cursor_y = fb_height - 1;
}

void cursor_draw(void) {
    // simple 8x8 white square cursor
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            fb_putpixel(cursor_x + x, cursor_y + y, 0xFFFFFF);
}

int cursor_get_x(void) {
    return cursor_x;
}

int cursor_get_y(void) {
    return cursor_y;
}
