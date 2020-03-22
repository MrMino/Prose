/*
 * This file is part of Prose Proxy project.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "fatal.h"

#define LOCAL_PORT 12345
#define MAX_LISTEN_BACKLOG 128

#define BUFFER_LEN 1024

/* TODO
 * 1. close() should be called after bind() or listen() fails.
 * 2. close() should be called on a SIGINT.
 * 3. recv() should be called until CRLFCRLF sequence is received.
 */


void serve(int client_socketfd){
    char *buffer = malloc(BUFFER_LEN);

    int bytes_read = recv(client_socketfd, buffer, BUFFER_LEN, 0);
    if (bytes_read < 1) {
        free(buffer);
        close(client_socketfd);
        return;
    }
    buffer[bytes_read] = '\0';

    char* url_start = buffer + 8;
    char* url_end = strstr(url_start, " HTTP/1.1\r\n");
    if (url_end == NULL || strncmp(buffer, "CONNECT ", 8) != 0) {
        free(buffer);
        close(client_socketfd);
        return;
    }

    int url_len = url_end - url_start + 1;
    char* url = malloc(url_len);
    strncpy(url, url_start, url_len);
    url[url_len] = '\0';

    printf("URL (len: %d): %s\n", url_len, url);

    free(url);
    free(buffer);
    close(client_socketfd);
}

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

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    for (;;) {
        int client = accept(
                socketfd,
                (struct sockaddr*) &client_addr,
                &client_addr_len
            );

        serve(client);
        break;
    }

    close(socketfd);
}
