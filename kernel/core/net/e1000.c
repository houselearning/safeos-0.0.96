#include "e1000.h"
#include "../pci.h"
#include "../../arch/x86/io.h"
#include "../../core/string.h"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

typedef struct {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
} e1000_tx_desc_t;

typedef struct {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} e1000_rx_desc_t;

static uint32_t e1000_base;
static e1000_tx_desc_t tx_descs[E1000_NUM_TX_DESC];
static e1000_rx_desc_t rx_descs[E1000_NUM_RX_DESC];
static uint8_t tx_bufs[E1000_NUM_TX_DESC][2048];
static uint8_t rx_bufs[E1000_NUM_RX_DESC][2048];

static uint32_t mmio_read32(uint32_t offset) {
    return *(volatile uint32_t *)(e1000_base + offset);
}

static void mmio_write32(uint32_t offset, uint32_t value) {
    *(volatile uint32_t *)(e1000_base + offset) = value;
}

int e1000_init(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t bar0 = pci_config_read32(bus, slot, func, 0x10);
    e1000_base = bar0 & ~0xF; // Assume identity mapped

    // Reset
    mmio_write32(0x0000, 1);
    while (mmio_read32(0x0000) & 1);

    // Set up transmit
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_descs[i].addr = (uint64_t)&tx_bufs[i];
        tx_descs[i].cmd = 0;
        tx_descs[i].status = 1; // DD
    }
    mmio_write32(0x3800, (uint32_t)&tx_descs); // TDBAL
    mmio_write32(0x3804, 0); // TDBAH
    mmio_write32(0x3808, E1000_NUM_TX_DESC * 16); // TDLEN
    mmio_write32(0x3810, 0); // TDH
    mmio_write32(0x3818, 0); // TDT

    // Set up receive
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_descs[i].addr = (uint64_t)&rx_bufs[i];
        rx_descs[i].status = 0;
    }
    mmio_write32(0x2800, (uint32_t)&rx_descs); // RDBAL
    mmio_write32(0x2804, 0); // RDBAH
    mmio_write32(0x2808, E1000_NUM_RX_DESC * 16); // RDLEN
    mmio_write32(0x2810, 0); // RDH
    mmio_write32(0x2818, E1000_NUM_RX_DESC - 1); // RDT

    // Enable
    mmio_write32(0x0100, 0x04000000 | 0x02000000 | 0x00000001); // RCTL
    mmio_write32(0x0400, 0x00000002 | 0x00000008); // TCTL

    return 0;
}

int e1000_send(const void *data, int len) {
    static int tx_index = 0;
    if (len > 2048) return -1;

    memcpy(tx_bufs[tx_index], data, len);
    tx_descs[tx_index].length = len;
    tx_descs[tx_index].cmd = 0x0B; // EOP, IFCS, RS
    tx_descs[tx_index].status = 0;

    uint32_t tdt = mmio_read32(0x3818);
    mmio_write32(0x3818, (tdt + 1) % E1000_NUM_TX_DESC);

    tx_index = (tx_index + 1) % E1000_NUM_TX_DESC;
    return len;
}

int e1000_poll(void *buf, int maxlen) {
    uint32_t rdt = mmio_read32(0x2818);
    uint32_t next = (rdt + 1) % E1000_NUM_RX_DESC;

    if (rx_descs[next].status & 0x01) { // DD
        int len = rx_descs[next].length;
        if (len > maxlen) len = maxlen;
        memcpy(buf, rx_bufs[next], len);
        rx_descs[next].status = 0;
        mmio_write32(0x2818, next);
        return len;
    }
    return 0;
}
