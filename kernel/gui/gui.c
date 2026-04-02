#include "gui.h"
#include "cursor.h"
#include "input.h"
#include "desktop.h"
#include "../arch/x86/framebuffer.h"

int gui_init(void) {
    /* Try to initialize the GUI. If there is no usable framebuffer
       available, return error so the kernel can fall back to a
       serial/idle mode rather than attempting graphics. */
    framebuffer_clear(0x202020);
    extern uint8_t* fb_address;
    /* If no framebuffer provided by bootloader, initialize fallback framebuffer. */
    if (!fb_address || (uintptr_t)fb_address == 0) {
        framebuffer_init(1024, 768, 32);
        if (fb_address && (uintptr_t)fb_address != 0) {
            return 0;
        }
        return -1;
    }
    return 0;
}

void gui_main_loop(void) {
    while (1) {
        gui_event_t ev;
        while (input_poll(&ev)) {
            desktop_handle_event(&ev);
        }
        desktop_draw();
        cursor_draw();
        /* Present rendered backbuffer to the hardware framebuffer */
        fb_present();
    }
}
