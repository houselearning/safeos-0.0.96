#include "arch/x86/interrupts.h"
#include "arch/x86/keyboard.h"
#include "arch/x86/mouse.h"
#include "arch/x86/framebuffer.h"
#include "core/memory.h"
#include "gui/gui.h"
#include "gui/desktop.h"

void kmain(unsigned long magic, unsigned long addr) {
    (void)magic;
    (void)addr;

    interrupts_init();
    memory_init();
    framebuffer_init(1024, 768, 32);   // assume VBE mode set by GRUB
    keyboard_init();
    mouse_init();

    gui_init();
    desktop_show_startup_screen("SafeOS 1.0", "Loading system modules...");
    // simulate loading
    for (int i = 0; i < 10000000; ++i) { __asm__ __volatile__("nop"); }

    desktop_init_home();   // icons: Notepad, Calculator, Spreadsheet, Files, Browser
    gui_main_loop();       // event loop
}
