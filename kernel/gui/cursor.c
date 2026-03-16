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

    /* Draw a simple arrow pointer 12x16 (white). Use a fixed-size mask
       encoded as 16-bit rows so the array size is compile-time constant. */
    const int W = 12, H = 16;
    static const uint16_t mask[16] = {
        0x8000, 0xC000, 0xE000, 0xF000,
        0xF800, 0xFC00, 0xFE00, 0xFF00,
        0xFE00, 0xEC00, 0xC800, 0x9000,
        0x2000, 0x0000, 0x0000, 0x0000
    };
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (mask[y] & (1u << (15 - x))) {
                fb_putpixel(cursor_x + x, cursor_y + y, 0xFFFFFF);
            }
        }
    }
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
