#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Initialize mouse
void mouse_init(void);

// Read mouse data
uint8_t mouse_read(void);

// Process mouse packet
void mouse_process_packet(uint8_t data);

#endif
