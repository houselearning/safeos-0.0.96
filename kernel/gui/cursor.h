#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>

// Current cursor position
extern int cursor_x;
extern int cursor_y;

// Initialize cursor system
void cursor_init(void);

// Update cursor position (called from mouse events)
void cursor_move(int dx, int dy);

// Draw the cursor on screen
void cursor_draw(void);

#endif
