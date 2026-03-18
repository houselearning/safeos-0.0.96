#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>

/* Sleep for microseconds */
int usleep(unsigned int useconds);

/* Sleep for seconds */
unsigned int sleep(unsigned int seconds);

/* Execute a shell command */
int system(const char *command);

#endif
