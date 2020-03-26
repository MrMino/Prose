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

#define LOCAL_PORT 12345
#define MAX_LISTEN_BACKLOG 128

#define BUFFER_LEN 1024

/* Read at most bytes_max data available in the socket.
 *
 * This is a simple wrapper around select() -> recv().
 * Received data is put into memory address pointed to by buffer argument.
 *
 * Returns the number of received bytes or -1 on error.
 *
 * If data is not received until specified timeout, sets errno to
 * REQUEST_TIMED_OUT and return -1.
 *
 * NOTE: this implementation assumes Linux behavior of select() in regards to
 * timeout. The timeval struct is therefore modified in-place after this call,
 * to reflect the time left after select() returns.
 */
#define REQUEST_TIMED_OUT EWOULDBLOCK
int timed_read(int sockfd, char *buffer, int bytes_max,
               struct timeval *timeout) {
    fd_set single_sock;
    FD_ZERO(&single_sock);
    FD_SET(sockfd, &single_sock);

    int retval = select(sockfd + 1, &single_sock, NULL, NULL, timeout);
    if (retval < 0) {
        return -1;
    } else if (retval == 0) {
        errno = REQUEST_TIMED_OUT;
        return -1;
    } else {
        return recv(sockfd, buffer, bytes_max, 0);
    }
}

/* Read from socket until one of the following:
 *  - "\r\n\r\n" (double CRLF) sequence is received
 *  - received bytes_max bytes before double CRLF occured
 *  - timeout is exceeded
 *
 * Received data is put into memory address pointed to by buffer argument.
 * Buffer is NOT null terminated.
 *
 * NOTE: if additional bytes after double CRLF are received, they are put into
 * the buffer with the rest of the data.
 *
 * Returns number of bytes received, if double CRLF is in the buffer after
 * reading. Returns -1 on an error.
 *
 * Set errno to REQUEST_TOO_LONG if bytes_max is exceeded.
 * Set errno to REQUEST_TIMED_OUT if timeout is exceeded.
 */
#define REQUEST_TOO_LONG ENOMEM
int recv_request_head(int sockfd, char *buffer, int bytes_max,
                      struct timeval *timeout) {
    int byte_count = 0;
    char *buffer_tail = buffer;
    while (byte_count < bytes_max) {
        int bytes_read = timed_read(sockfd, buffer_tail,
                                    bytes_max - byte_count, timeout);
        byte_count += bytes_read;

        if (bytes_read < 0) {
            return -1;
        }
        if (bytes_read == 0) {
            return 0;
        }
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            return byte_count;
        }
        buffer_tail += bytes_read;
    }

    errno = REQUEST_TOO_LONG;
    return -1;
}

/* TODO
 * 1. close() should be called after bind() or listen() fails.
 * 2. close() should be called on a SIGINT.
 * 3. recv() should be called until CRLFCRLF sequence is received.
 */

void serve(int client_socketfd) {
    char *buffer = malloc(BUFFER_LEN);

    int bytes_read = recv(client_socketfd, buffer, BUFFER_LEN, 0);
    if (bytes_read < 1) {
        free(buffer);
        close(client_socketfd);
        return;
    }
    buffer[bytes_read] = '\0';

    char *url_start = buffer + 8;
    char *url_end = strstr(url_start, " HTTP/1.1\r\n");
    if (url_end == NULL || strncmp(buffer, "CONNECT ", 8) != 0) {
        free(buffer);
        close(client_socketfd);
        return;
    }

    int url_len = url_end - url_start + 1;
    char *url = malloc(url_len);
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

    if (bind(socketfd, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr_in))
        < 0) {
        fatal_errno("Cannot bind", FATAL_SOCK_INIT_FAIL);
    }

    if (listen(socketfd, MAX_LISTEN_BACKLOG) < 0) {
        fatal_errno("Cannot listen on server socket", FATAL_SOCK_INIT_FAIL);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    for (;;) {
        int client = accept(socketfd, (struct sockaddr *)&client_addr,
                            &client_addr_len);

        serve(client);
        break;
    }

    close(socketfd);
}
