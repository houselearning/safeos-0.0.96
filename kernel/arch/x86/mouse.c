#include "mouse.h"
#include "io.h"
#include "../../gui/input.h"

static int mouse_cycle = 0;
static uint8_t mouse_packet[3];

void mouse_init(void) {
    // Enable mouse interrupt
    outb(0x21, inb(0x21) & ~0x10); // Clear bit 4 (IRQ12)

    // Enable auxiliary device
    outb(0x64, 0xA8);

    // Enable interrupts
    outb(0x64, 0x20);
    uint8_t status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    // Set defaults
    outb(0x64, 0xD4);
    outb(0x60, 0xF6);
    mouse_read(); // ACK

    // Enable data reporting
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    mouse_read(); // ACK
}

uint8_t mouse_read(void) {
    while (!(inb(0x64) & 1));
    return inb(0x60);
}

void mouse_process_packet(uint8_t data) {
    mouse_packet[mouse_cycle++] = data;
    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        uint8_t buttons = mouse_packet[0] & 0x07;
        int8_t dx = mouse_packet[1];
        int8_t dy = mouse_packet[2];

        input_handle_mouse(dx, -dy);
        input_handle_mouse_buttons(buttons);
    }
}
