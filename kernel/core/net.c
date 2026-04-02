#include "net.h"
#include "string.h"
#include "stdio.h"
#include <stdbool.h>

void net_init(void) {
    // Basic initialization placeholder: record that networking subsystem
    // has been initialized. Real driver initialization happens elsewhere.
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
}

// Attempt to perform a very small, self-contained HTTP GET operation.
// This implementation does not rely on a full TCP/IP stack provided
// by userland; instead it performs basic input validation and returns
// a clear error when networking functionality is not available.
// Return: >=0 number of bytes written into buffer on success, -1 on error.
int net_http_get(const char *url, char *buffer, int maxlen) {
    if (!url || !buffer || maxlen <= 0) return -1;

    // Only accept very small local-scheme fetches that the kernel can
    // service without a full TCP stack. Supported schemes:
    // - "about:blank" => simple HTML
    // Otherwise return error to indicate network unavailable.

    const char *about = "about:blank";
    if (strcmp(url, about) == 0) {
        const char *page = "<html><body><h1>About Blank</h1></body></html>";
        int len = strlen(page);
        if (len >= maxlen) len = maxlen - 1;
        memcpy(buffer, page, len);
        buffer[len] = '\0';
        return len;
    }

    // If any network stack is present, a symbol `net_socket_connect` would
    // be provided by drivers. Check weak symbol linkage (compile-time).
#if defined(HAVE_KERNEL_SOCKETS)
    // If the kernel was built with socket support, use it. This block is
    // intentionally minimal to avoid linking heavy dependencies when
    // sockets are not available.
    // Attempt to parse "http://host[:port]/path"
    char host[128] = {0};
    int port = 80;
    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) p += 7;
    else return -1;

    const char *slash = strchr(p, '/');
    int hostlen = slash ? (int)(slash - p) : (int)strlen(p);
    if (hostlen <= 0 || hostlen >= (int)sizeof(host)) return -1;
    memcpy(host, p, hostlen);
    host[hostlen] = '\0';

    // TODO: name resolution and TCP connect would go here. At present
    // we return -1 to indicate the requested URL cannot be fetched.
    (void)host; (void)port;
    return -1;
#else
    // Networking not available in this build
    return -1;
#endif
}