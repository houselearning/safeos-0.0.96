#include "stdio.h"
#include "string.h"

static void print_char(char **out, char c) {
    **out = c;
    (*out)++;
}

int sprintf(char *out, const char *fmt, ...) {
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

int snprintf(char *out, size_t size, const char *fmt, ...) {
    // simple wrapper — you can expand later
    return sprintf(out, fmt);
}

int kprintf(const char *fmt, ...) {
    // optional: print to serial or framebuffer
    return 0;
}

int sscanf(const char *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = 0;

    while (*fmt && *str) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                int *p = va_arg(args, int *);
                int sign = 1;
                if (*str == '-') { sign = -1; str++; }
                *p = 0;
                while (*str >= '0' && *str <= '9') {
                    *p = *p * 10 + (*str - '0');
                    str++;
                }
                *p *= sign;
                count++;
            }
        } else if (*fmt == *str) {
            fmt++; str++;
        } else {
            break;
        }
    }
    va_end(args);
    return count;
}
