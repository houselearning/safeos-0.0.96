#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include "../gui_event.h"
#include "../window.h"

// Open the file explorer window
void file_explorer_open(const char *path);

// Handle mouse/keyboard events
void file_explorer_handle_event(gui_event_t *ev);

// Draw the file explorer window
void file_explorer_draw(void);

#endif
