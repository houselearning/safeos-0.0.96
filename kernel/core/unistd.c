#include "unistd.h"

/* Simple busy-wait implementation for kernel environment */
/* In a real OS, this would use timer interrupts */

#define LOOPS_PER_MICROSECOND 100  /* Adjust based on CPU speed */

int usleep(unsigned int useconds) {
    /* Simple busy-wait loop */
    for (unsigned int i = 0; i < useconds; i++) {
        for (int j = 0; j < LOOPS_PER_MICROSECOND; j++) {
            __asm__ volatile("nop");  /* Prevent optimization */
        }
    }
    return 0;
}

unsigned int sleep(unsigned int seconds) {
    /* Convert seconds to microseconds and call usleep */
    usleep(seconds * 1000000);
    return 0;
}

int system(const char *command) {
    /* In kernel environment, typically not available */
    /* Return -1 to indicate failure */
    (void)command;  /* Suppress unused warning */
    return -1;
}
