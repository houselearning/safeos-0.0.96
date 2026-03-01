#include "e1000.h"

int e1000_init(uint8_t bus, uint8_t slot, uint8_t func) {
    (void)bus; (void)slot; (void)func;
    return 0;
}

int e1000_send(const void *data, int len) {
    (void)data; (void)len;
    return 0;
}

int e1000_poll(void *buf, int maxlen) {
    (void)buf; (void)maxlen;
    return 0;
}
