#include "framebuffer.h"

uint8_t* fb_address = 0;
uint32_t fb_width = 0;
uint32_t fb_height = 0;
uint32_t fb_pitch = 0;
uint32_t fb_bpp = 0;

void fb_init(uint8_t* address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    fb_address = address;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
}

void fb_clear(uint32_t color) {
    uint32_t* fb = (uint32_t*)fb_address;
    for (uint32_t y = 0; y < fb_height; ++y) {
        for (uint32_t x = 0; x < fb_width; ++x) {
            fb[y * (fb_pitch / 4) + x] = color;
        }
    }
}

void fb_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb_width || y >= fb_height) return;
    uint32_t* fb = (uint32_t*)fb_address;
    fb[y * (fb_pitch / 4) + x] = color;
}
