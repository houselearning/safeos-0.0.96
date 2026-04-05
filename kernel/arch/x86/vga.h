#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/* Set VGA 320x200x256 color mode (mode 13h) using direct VGA registers.
   Returns 0 on success, non-zero on failure. */
int vga_set_mode_13h(void);

/* Simple operations for this mode */
void vga13_clear(uint8_t color);
void vga13_putpixel(int x, int y, uint8_t color);

#endif