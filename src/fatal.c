/*
 * This file is part of Prose Proxy project.
 *
 * See fatal.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "fatal.h"

const exit_code FATAL_SOCK_INIT_FAIL = 1;

void fatal_exit(const char *message, exit_code status) {
    fprintf(stderr, "%s\n", message);
    exit(status);
}

void fatal_errno(const char *message, exit_code status) {
    perror(message);
    exit(status);
}
