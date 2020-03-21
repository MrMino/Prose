/*
 * This file is part of Prose Proxy project.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "fatal.h"

int main() {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        fatal_errno("Cannot create a socket", FATAL_SOCK_INIT_FAIL);
    }


    close(socketfd);
}
