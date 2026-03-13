#include "net.h"
#include "string.h"

void net_init(void) {
    // Initialize networking stack
}

int net_http_get(const char *url, char *buffer, int maxlen) {
    // Stub: return a fake HTML page
    const char *fake_page = "<html><body><h1>Welcome to SafeOS Browser</h1><p>This is a fake webpage.</p></body></html>";
    int len = strlen(fake_page);
    if (len >= maxlen) len = maxlen - 1;
    memcpy(buffer, fake_page, len);
    buffer[len] = '\0';
    return len;
}