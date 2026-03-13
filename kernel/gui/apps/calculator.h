#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "../gui_event.h"
#include "../window.h"

void calculator_open(const char *path);
void calculator_handle_event(gui_event_t *ev);
void calculator_draw(void);

#endif
