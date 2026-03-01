#ifndef BROWSER_H
#define BROWSER_H

#include "../gui_event.h"
#include "../window.h"


/* Open a new browser window (new tab behavior) */
void browser_open(void);

/* Handle mouse + keyboard events */
void browser_handle_event(gui_event_t *ev);

/* Draw browser UI */
void browser_draw(void);

#endif /* BROWSER_H */