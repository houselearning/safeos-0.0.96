#ifndef E1000_H
#define E1000_H

#include <stdint.h>

int e1000_init(uint8_t bus, uint8_t slot, uint8_t func);
int e1000_send(const void *data, int len);
int e1000_poll(void *buf, int maxlen);

#endif
