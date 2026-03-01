#include "gui.h"
#include "cursor.h"
#include "input.h"
#include "desktop.h"
#include "../arch/x86/framebuffer.h"

void gui_init(void) {
    framebuffer_clear(0x202020);
}

void gui_main_loop(void) {
    while (1) {
        gui_event_t ev;
        while (input_poll(&ev)) {
            desktop_handle_event(&ev);
        }
        desktop_draw();
        cursor_draw();
    }
}
