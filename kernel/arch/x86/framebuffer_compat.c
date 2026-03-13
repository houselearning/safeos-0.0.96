#include "framebuffer.h"
#include <stdint.h>

/*
 * Provide a simple in-kernel fallback framebuffer so the GUI can run
 * even when the bootloader didn't supply a linear framebuffer.
 *
 * This uses a statically allocated buffer in kernel memory and
 * initializes the framebuffer metadata to point at it.
 */
#define FALLBACK_WIDTH 1024
#define FALLBACK_HEIGHT 768

static uint32_t fallback_fb[FALLBACK_WIDTH * FALLBACK_HEIGHT];

void framebuffer_init(int width, int height, int bpp) {
    (void)width; (void)height; (void)bpp;
    extern uint8_t* fb_address;
    /* If a real framebuffer was initialized already, keep it. */
    if (fb_address) return;

    /* Initialize fb metadata to our static buffer (32bpp, tightly packed). */
    fb_init((uint8_t *)fallback_fb, FALLBACK_WIDTH, FALLBACK_HEIGHT,
            FALLBACK_WIDTH * 4, 32);
}

void framebuffer_clear(uint32_t color) {
    fb_clear(color);
}
