#include "cursor.h"
#include "arch/x86/framebuffer.h"

static int cursor_x = 100;
static int cursor_y = 100;

static const uint8_t cursor_bitmap[16][16] = {
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // ... fill triangle ...
};

void cursor_move(int dx, int dy) {
    cursor_x += dx;
    cursor_y += dy;
    if (cursor_x < 0) cursor_x = 0;
    if (cursor_y < 0) cursor_y = 0;
}

void cursor_draw(void) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            if (cursor_bitmap[y][x]) {
                framebuffer_putpixel(cursor_x + x, cursor_y + y, 0xFFFFFFFF);
            }
        }
    }
}
