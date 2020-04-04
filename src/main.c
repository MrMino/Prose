/*
 * This file is part of Prose Proxy project.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <errno.h>

#include "fatal.h"
#include "string_utils.h"
#include "requests.h"
#include "transport.h"

#define LOCAL_PORT 12345
#define MAX_LISTEN_BACKLOG 128

#define BUFFER_LEN 1024 * 8

void handle_request(int socketfd, char *buffer) {
    struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
    int received_bytes =
            recv_request_head(socketfd, buffer, BUFFER_LEN - 1, &timeout);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0';

        char *method_line = copy_line_crlf(buffer);
        if (method_line == NULL) {
            if (errno == 0) {
                puts("BAD REQUEST LINE");
            } else {
                puts("ERROR PARSING REQUEST LINE");
            }
            return;
        }

        http_request_line *request = parse_request_line(method_line);
        if (request == NULL) {
            if (errno == 0) {
                puts("BAD REQUEST LINE");
            } else {
                puts("ERROR PARSING REQUEST LINE");
            }
            return;
        }

        printf("Method: %s\nURI: %s\nVersion: %s\n", request->method,
               request->uri, request->http_version);
        free_http_request_line(request);
    } else {
        puts("TIMED OUT");
    }
}

/* TODO
 * close() should be called on a SIGINT.
 * After first request, use splice() loop.
 */

void serve_client(int client_socketfd) {
    char *buffer = malloc(BUFFER_LEN);

    handle_request(client_socketfd, buffer);

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

    if (bind(socketfd, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr_in))
        < 0) {
        close(socketfd);
        fatal_errno("Cannot bind", FATAL_SOCK_INIT_FAIL);
    }

    if (listen(socketfd, MAX_LISTEN_BACKLOG) < 0) {
        close(socketfd);
        fatal_errno("Cannot listen on server socket", FATAL_SOCK_INIT_FAIL);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    for (;;) {
        int client = accept(socketfd, (struct sockaddr *)&client_addr,
                            &client_addr_len);

        serve_client(client);
    }

    close(socketfd);
}
