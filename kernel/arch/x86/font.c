#include "font.h"

const uint8_t font_arial[128][8] = {
    // 0-31: control
    {0,0,0,0,0,0,0,0},
    // 32: space
    {0,0,0,0,0,0,0,0},
    // 33: !
    {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    // ... add more, for now stub
};

const uint8_t font_times[128][8] = {
    // Similar
};

const uint8_t (*current_font)[8] = font_arial;

void font_set_arial(void) {
    current_font = font_arial;
}

void font_set_times(void) {
    current_font = font_times;
}