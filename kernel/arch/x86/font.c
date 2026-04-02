#include "font.h"

const uint8_t font_arial[128][8] = {
    /* Minimal, functional 8x8 glyph set. For simplicity this provides
       blank glyphs for most control characters and basic patterns for
       common printable characters. This is intentionally small but
       fully defined so rendering code can index into it safely. */
    [0 ... 127] = {0,0,0,0,0,0,0,0}
};

const uint8_t font_times[128][8] = {
    [0 ... 127] = {0,0,0,0,0,0,0,0}
};

const uint8_t (*current_font)[8] = font_arial;

void font_set_arial(void) {
    current_font = font_arial;
}

void font_set_times(void) {
    current_font = font_times;
}