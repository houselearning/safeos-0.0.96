#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include "gui_event.h"

// Colors used by the GUI
#define GUI_COLOR_TEXT      0xFFFFFF
#define GUI_COLOR_HIGHLIGHT 0x404080
#define GUI_COLOR_MENU      0x303030

// Mouse buttons
#define GUI_MOUSE_LEFT   1
#define GUI_MOUSE_RIGHT  2

// Event types
#define GUI_EVENT_MOUSE_DOWN 1
#define GUI_EVENT_MOUSE_UP   2
#define GUI_EVENT_KEY_DOWN   3
#define GUI_EVENT_KEY_UP     4
// Character input event (text input)
#define GUI_EVENT_KEY_CHAR   5

// Utility functions
int gui_point_in_rect(int px, int py, int x, int y, int w, int h);

// Drawing functions
void gui_draw_rect(int x, int y, int w, int h, uint32_t color);
void gui_draw_text(int x, int y, const char *text, uint32_t color);

// Additional colors
#define GUI_COLOR_BACKGROUND 0x101010

// Key codes (simple ASCII/controls)
#define GUI_KEY_BACKSPACE 8
#define GUI_KEY_ENTER     13

#endif
