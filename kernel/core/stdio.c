#include "stdio.h"
#include "string.h"

static void print_char(char **out, char c) {
    **out = c;
    (*out)++;
}

int ksprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char *p = out;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char *s = va_arg(args, const char *);
                while (*s) print_char(&p, *s++);
            } else if (*fmt == 'd') {
                int v = va_arg(args, int);
                char buf[32];
                int i = 0;
                if (v < 0) { print_char(&p, '-'); v = -v; }
                do { buf[i++] = '0' + (v % 10); v /= 10; } while (v);
                while (i--) print_char(&p, buf[i]);
            }
        } else {
            print_char(&p, *fmt);
        }
        fmt++;
    }

    *p = '\0';
    va_end(args);
    return p - out;
}

int ksnprintf(char *out, size_t size, const char *fmt, ...) {
    // simple wrapper — you can expand later
    return ksprintf(out, fmt);
}

int kprintf(const char *fmt, ...) {
    // optional: print to serial or framebuffer
    return 0;
}
