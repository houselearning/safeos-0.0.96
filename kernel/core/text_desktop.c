#include <stdint.h>
#include "text_desktop.h"

static volatile uint16_t *vga = (uint16_t *)0xB8000;

static void vga_puts(int row, int col, const char *s, uint8_t attr) {
    int off = row * 80 + col;
    while (*s && off < 80*25) {
        uint8_t ch = (uint8_t)*s++;
        vga[off++] = (uint16_t)((attr << 8) | ch);
    }
}

void text_desktop_run(void) {
    /* Clear screen */
    for (int i = 0; i < 80*25; ++i) vga[i] = (uint16_t)((0x07 << 8) | ' ');

    vga_puts(1, 1, "SafeOS 1.0 (text desktop)", 0x1E);
    vga_puts(3, 2, "Graphics framebuffer not available.", 0x07);
    vga_puts(5, 2, "Keyboard and mouse not yet supported in this fallback.", 0x07);
    vga_puts(7, 2, "If you see this, the kernel reached the desktop fallback.", 0x07);
    vga_puts(9, 2, "Press Ctrl+Alt to grab/release the mouse in your VM.", 0x07);
    vga_puts(11, 2, "Think you know the fix? https://github.com/houselearning/safeos-0.0.96", 0x07);
    /* Halt here to keep the screen visible. Use `hlt` without `cli`
       so the guest doesn't disable interrupts then halt (which some
       hypervisors treat as a shutdown request). */
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

/* Read a byte from an I/O port */
static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

/* Attempt to reboot via keyboard controller; hang if it fails. */
static void reboot_via_kb(void) {
    unsigned char status;
    __asm__ __volatile__("cli");
    /* wait for input buffer clear */
    for (int i = 0; i < 100000; ++i) {
        status = inb(0x64);
        if (!(status & 0x02)) break;
    }
    __asm__ __volatile__("outb %0, %1" :: "a"((unsigned char)0xFE), "Nd"((unsigned short)0x64));
    for (;;) __asm__ __volatile__("hlt");
}

/* Show a Blue Screen Of Death style error screen in VGA text mode.
   Displays an ASCII smiley, a message, 100 error codes/log lines,
   and accepts F7 to restart. */
void bsod_show(const char *error_code) {
    /* Clear screen and set blue background (0x1F = white on blue) */
    for (int i = 0; i < 80*25; ++i) vga[i] = (uint16_t)((0x1F << 8) | ' ');

    /* Large ASCII smiley upper-left */
    vga_puts(1, 2, ":)", 0x1F);

    vga_puts(3, 2, "We've encountered an error. The machine will now restart.", 0x1F);
    if (error_code) {
        vga_puts(5, 2, "Error code:", 0x1F);
        vga_puts(5, 14, error_code, 0x1F);
    }

    /* Show 100 error log lines starting at row 7 */
    char buf[32];
    for (int i = 0; i < 100; ++i) {
        int row = 7 + (i % 18); /* leave room for bottom message */
        int col = 2 + (i / 18) * 24; /* wrap into columns if too many */
        unsigned int code = 0xDEAD0000u + (unsigned int)i;
        /* simple integer to hex formatting into buf */
        const char *hex = "0123456789ABCDEF";
        buf[0] = '\0';
        int p = 0;
        buf[p++] = '0'; buf[p++] = 'x';
        for (int n = 7; n >= 0; --n) {
            buf[p++] = hex[(code >> (n*4)) & 0xF];
        }
        buf[p++] = '\0';
        vga_puts(row, col, buf, 0x1F);
    }

    /* Bottom instruction */
    vga_puts(24, 2, "PRESS F7 TO RESTART", 0x1F);

    /* Wait for F7 key (scancode 0x41 for make code in set 1) */
    for (;;) {
        unsigned char sc = inb(0x60);
        if (sc == 0x41) {
            reboot_via_kb();
        }
        /* halt to reduce CPU usage */
        __asm__ __volatile__("hlt");
    }
}
