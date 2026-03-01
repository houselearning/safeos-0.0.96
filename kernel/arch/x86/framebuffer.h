#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

// Framebuffer properties (set by your bootloader or initialization code)
extern uint8_t* fb_address;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t fb_bpp;

// Initialize framebuffer (you implement this in framebuffer.c)
void fb_init(uint8_t* address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp);

// Draw a pixel
void fb_putpixel(uint32_t x, uint32_t y, uint32_t color);

// Clear screen
void fb_clear(uint32_t color);

// Compatibility wrappers used by higher-level GUI code
// Initialize framebuffer with width/height/bpp when address unknown
void framebuffer_init(int width, int height, int bpp);

// Compatibility clear wrapper
void framebuffer_clear(uint32_t color);

// Draw a character (optional, if you have a font)
void fb_putchar(int x, int y, char c, uint32_t fg, uint32_t bg);

#endif
