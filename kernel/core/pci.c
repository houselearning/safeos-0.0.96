#include "pci.h"
#include "../arch/x86/io.h"   // inb/outb/inl/outl
#include "net/e1000.h"        // your NIC driver

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address =
        (uint32_t)((bus << 16) | (slot << 11) |
                   (func << 8) | (offset & 0xFC) | 0x80000000);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t data = pci_read32(bus, slot, func, offset);
    return (data >> ((offset & 2) * 8)) & 0xFFFF;
}

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return pci_read32(bus, slot, func, offset);
}

void pci_scan(void) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor = pci_config_read16(bus, slot, 0, 0x00);
            if (vendor == 0xFFFF)
                continue;

            uint16_t device = pci_config_read16(bus, slot, 0, 0x02);

            // Intel E1000 family
            if (vendor == 0x8086 &&
               (device == 0x100E || device == 0x100F || device == 0x10D3)) {

                e1000_init(bus, slot, 0);
            }
        }
    }
}
