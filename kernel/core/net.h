#ifndef NET_H
#define NET_H

#include <stdint.h>

// Initialize networking subsystem
void net_init(void);

// Perform a simple HTTP GET request
// Returns number of bytes written into buffer, or -1 on failure
int net_http_get(const char *url, char *buffer, int maxlen);

#endif
