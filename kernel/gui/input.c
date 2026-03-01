#include "input.h"
#include "gui_event.h"

// Minimal stub: no input available, so always return 0 events
int input_poll(gui_event_t *out) {
    (void)out;
    return 0;
}
