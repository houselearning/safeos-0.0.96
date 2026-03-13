#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Font data for Arial-like
extern const uint8_t font_arial[128][8];

// Font data for Times-like
extern const uint8_t font_times[128][8];

// Current font
extern const uint8_t (*current_font)[8];

void font_set_arial(void);
void font_set_times(void);

#endif