#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "gui_event.h"

// Initialize input system (keyboard + mouse)
void input_init(void);

// Handle raw keyboard scancode
void input_handle_key(uint8_t scancode, int pressed);

// Handle raw mouse packet (dx, dy, buttons)
void input_handle_mouse(int dx, int dy, int buttons);

// Dispatch queued events to the desktop/window manager
void input_dispatch(void);

#endif
