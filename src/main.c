/*
 * This file is part of Prose Proxy project.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "fatal.h"

#define LOCAL_PORT 12345
#define MAX_LISTEN_BACKLOG 128

int main() {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        fatal_errno("Cannot create a socket", FATAL_SOCK_INIT_FAIL);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(LOCAL_PORT);

    if (bind(socketfd, (struct sockaddr*) &server_addr,
                sizeof(struct sockaddr_in)) < 0) {
        fatal_errno("Cannot bind", FATAL_SOCK_INIT_FAIL);
    }

    if (listen(socketfd, MAX_LISTEN_BACKLOG) < 0) {
        fatal_errno("Cannot listen on server socket", FATAL_SOCK_INIT_FAIL);
    }

    close(socketfd);
}
