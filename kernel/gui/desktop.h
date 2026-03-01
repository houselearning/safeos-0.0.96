#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "window.h"

// Forward declaration of GUI event type
typedef struct gui_event {
    int type;
    int x;
    int y;
    int button;
    int key;
} gui_event_t;

// Show startup splash screen
void desktop_show_startup_screen(const char *title, const char *msg);

// Initialize the desktop home screen
void desktop_init_home(void);

// Handle mouse/keyboard events
void desktop_handle_event(gui_event_t *ev);

// Draw desktop (icons, windows, menus)
void desktop_draw(void);

#endif
