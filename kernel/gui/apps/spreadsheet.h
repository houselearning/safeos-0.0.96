#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "../gui_event.h"
#include "../window.h"

// Open spreadsheet window
void spreadsheet_open(const char *path);

// Handle mouse/keyboard events
void spreadsheet_handle_event(gui_event_t *ev);

// Draw spreadsheet window
void spreadsheet_draw(void);

#endif
