#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <stdint.h>
#include "../window.h"
#include "../gui_event.h"


// Open a notepad window and load a file
void notepad_open(const char *path);

// Handle keyboard/mouse events
void notepad_handle_event(gui_event_t *ev);

// Draw the notepad window
void notepad_draw(void);

#endif
