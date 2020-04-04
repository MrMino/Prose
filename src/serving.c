/*
 * This file is part of Prose Proxy project.
 *
 * See serving.h.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

#include "serving.h"
#include "string_utils.h"
#include "requests.h"
#include "transport.h"

#define BUFFER_LEN 1024 * 8

void handle_request(int socketfd, char *buffer);

void serve_client(int client_socketfd) {
    char *buffer = malloc(BUFFER_LEN);

    handle_request(client_socketfd, buffer);

    free(buffer);
    close(client_socketfd);
}

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
