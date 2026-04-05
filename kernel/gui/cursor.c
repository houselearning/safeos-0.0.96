#include "cursor.h"
#include "../arch/x86/framebuffer.h"

int cursor_x = 100;
int cursor_y = 100;
int cursor_visible = 1;

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
    if (!cursor_visible) return;

    /* Draw a visible pointer cross at cursor location */
    const int SIZE = 8;
    for (int dy = -SIZE; dy <= SIZE; ++dy) {
        int y = cursor_y + dy;
        if (y < 0 || y >= (int)fb_height) continue;
        int x = cursor_x;
        if (x >= 0 && x < (int)fb_width) fb_putpixel(x, y, 0x00FF00);
    }
    for (int dx = -SIZE; dx <= SIZE; ++dx) {
        int x = cursor_x + dx;
        if (x < 0 || x >= (int)fb_width) continue;
        int y = cursor_y;
        if (y >= 0 && y < (int)fb_height) fb_putpixel(x, y, 0x00FF00);
    }

    /* Add a central dot to make it obvious */
    fb_putpixel(cursor_x, cursor_y, 0xFF0000);
}

int cursor_get_x(void) {
    return cursor_x;
}

int cursor_get_y(void) {
    return cursor_y;
}

void cursor_hide(void) { cursor_visible = 0; }
void cursor_show(void) { cursor_visible = 1; }
void cursor_toggle(void) { cursor_visible = !cursor_visible; }
