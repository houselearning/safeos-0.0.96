#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Initialize keyboard
void keyboard_init(void);

// Read scancode
uint8_t keyboard_read_scancode(void);

// Convert scancode to ASCII (basic)
char scancode_to_ascii(uint8_t scancode);

#endif
