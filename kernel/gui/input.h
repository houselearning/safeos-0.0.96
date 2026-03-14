#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "gui_event.h"

// Initialize input system (keyboard + mouse)
void input_init(void);

// Handle raw keyboard scancode
void input_handle_key(uint8_t scancode, int pressed);

// Handle raw mouse movement
void input_handle_mouse(int dx, int dy);

// Handle mouse button changes
void input_handle_mouse_buttons(int buttons);

// Dispatch queued events to the desktop/window manager
void input_dispatch(void);

// Poll for a single GUI event. Returns non-zero if `out` was filled.
int input_poll(gui_event_t *out);

#endif
