#include "gui.h"
#include "cursor.h"
#include "input.h"
#include "desktop.h"
#include "apps/notepad.h"
#include "apps/calculator.h"
#include "apps/spreadsheet.h"
#include "apps/file_explorer.h"
#include "apps/browser.h"
#include "../arch/x86/framebuffer.h"
#include "../core/text_desktop.h"
#include "../core/stdio.h"

int gui_init(void) {
    /* Try to initialize the GUI. If there is no usable framebuffer
       available, display a text-mode fallback and report failure. */
    extern uint8_t* fb_address;

    /* Validate framebuffer state. */
    int fb_valid = (fb_address && fb_width > 0 && fb_height > 0 && fb_pitch > 0 && (fb_bpp == 32 || fb_bpp == 24 || fb_bpp == 16 || fb_bpp == 8));
    if (!fb_valid) {
        serial_puts("GUI: framebuffer invalid or unsupported mode; forcing 1024x768x32\n");
        fb_address = NULL;
        fb_bpp = 0;
        framebuffer_init(1024, 768, 32);
        fb_valid = (fb_address && fb_width > 0 && fb_height > 0 && fb_pitch > 0 && fb_bpp == 32);
    }

    if (!fb_valid) {
        // Fallback handled in desktop_draw
    }

    framebuffer_alloc_backbuffer();
    cursor_init();
    /* Ensure desktop starts drawing immediately on first loop. */
    desktop_init_home();
    return 0;
}

void gui_main_loop(void) {
    serial_puts("GUI main loop active - desktop displayed\n");
    while (1) {
        gui_event_t ev;
        while (input_poll(&ev)) {
            desktop_handle_event(&ev);
        }
        desktop_draw();
        /* Draw all app windows on top of desktop */
        notepad_draw();
        calculator_draw();
        spreadsheet_draw();
        file_explorer_draw();
        browser_draw();
        cursor_draw();
        /* Present rendered backbuffer to the hardware framebuffer */
        fb_present();
    }
}
