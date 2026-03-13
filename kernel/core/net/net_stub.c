#include "../net.h"
#include "../string.h"

// Minimal HTTP GET stub — writes a placeholder message into out
// int net_http_get(const char *url, char *out, int maxlen) {
//     (void)url;
//     const char *msg = "[net_http_get not implemented]";
//     int i = 0;
//     while (msg[i] && i < maxlen - 1) { out[i] = msg[i]; i++; }
//     if (maxlen > 0) out[i] = '\0';
//     return i;
// }
