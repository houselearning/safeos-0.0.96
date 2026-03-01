#include "framebuffer.h"

// Compatibility wrappers used by higher-level code
void framebuffer_init(int width, int height, int bpp) {
    // In this minimal environment we don't have an address to pass;
    // initialize framebuffer metadata conservatively.
    fb_init(0, width, height, width * 4, bpp);
}

void framebuffer_clear(uint32_t color) {
    fb_clear(color);
}
