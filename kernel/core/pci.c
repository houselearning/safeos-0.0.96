#include "pci.h"
#include "../arch/x86/io.h"   // inb/outb/inl/outl
#include "net/e1000.h"        // your NIC driver
#include "stdio.h"            // serial diagnostics

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/* Simple serial diagnostics for PCI scan */
static void pci_serial_puts(const char *s) {
    while (*s) {
        __asm__ __volatile__("outb %%al, %%dx" :: "a"(*s), "d"((unsigned short)0x3f8));
        s++;
    }
}

static void pci_serial_puthex(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        __asm__ __volatile__("outb %%al, %%dx" :: "a"(hex[nib]), "d"((unsigned short)0x3f8));
    }
}

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address =
        (uint32_t)((bus << 16) | (slot << 11) |
                   (func << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t data = pci_read32(bus, slot, func, offset & 0xFC);
    return (data >> ((offset & 3) * 8)) & 0xFF;
}

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t data = pci_read32(bus, slot, func, offset & 0xFC);
    return (data >> ((offset & 2) * 8)) & 0xFFFF;
}

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return pci_read32(bus, slot, func, offset & 0xFC);
}

void pci_scan(void) {
    pci_serial_puts("PCI: starting bus scan...\n");
    int device_count = 0;
    
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            /* Try to read vendor ID from this PCI slot */
            uint16_t vendor = 0xFFFF;
            
            /* Use safe read with error handling */
            vendor = pci_config_read16(bus, slot, 0, 0x00);
            
            if (vendor == 0xFFFF)
                continue;  /* No device at this slot */

            uint16_t device = pci_config_read16(bus, slot, 0, 0x02);
            
            pci_serial_puts("PCI: found device at bus=0x");
            pci_serial_puthex(bus);
            pci_serial_puts(" slot=0x");
            pci_serial_puthex(slot);
            pci_serial_puts(" vendor=0x");
            pci_serial_puthex(vendor);
            pci_serial_puts(" device=0x");
            pci_serial_puthex(device);
            pci_serial_puts("\n");

            /* Intel E1000 family - attempt NIC initialization */
            if (vendor == 0x8086 &&
               (device == 0x100E || device == 0x100F || device == 0x10D3)) {
                pci_serial_puts("PCI: initializing Intel E1000 NIC...\n");
                int result = e1000_init(bus, slot, 0);
                if (result == 0) {
                    pci_serial_puts("PCI: E1000 initialized successfully\n");
                    device_count++;
                } else {
                    pci_serial_puts("PCI: E1000 initialization failed\n");
                }
            }
        }
    }
    
    pci_serial_puts("PCI: scan complete. Found ");
    pci_serial_puthex(device_count);
    pci_serial_puts(" device(s).\n");
}
