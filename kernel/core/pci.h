// pci.h
#ifndef PCI_H
#define PCI_H

#include <stdint.h>

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void pci_scan(void);

#endif
