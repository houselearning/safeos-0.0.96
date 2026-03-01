#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "window.h"
#include "gui_event.h"


// Show startup splash screen
void desktop_show_startup_screen(const char *title, const char *msg);

// Initialize the desktop home screen
void desktop_init_home(void);

// Handle mouse/keyboard events
void desktop_handle_event(gui_event_t *ev);

// Draw desktop (icons, windows, menus)
void desktop_draw(void);

#endif
