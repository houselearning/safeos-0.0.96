#ifndef ETH_H
#define ETH_H

#include <stdint.h>

void net_receive(const void *data, int len);   // called by e1000_poll
int  net_send_ipv4(uint32_t dst_ip, uint8_t proto,
                   const void *payload, int len);

void net_set_ip(uint32_t ip, uint32_t gateway, uint32_t mask);

#endif
