/*
 * This file is part of Prose Proxy project.
 *
 * See transport.h
 */

#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <string.h>

#include "transport.h"

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

int recv_request_head(int sockfd, char *buffer, int bytes_max,
                      struct timeval *timeout) {
    int byte_count = 0;
    char *buffer_tail = buffer;
    while (byte_count < bytes_max) {
        int bytes_read = timed_read(sockfd, buffer_tail,
                                    bytes_max - byte_count, timeout);

        if (bytes_read < 0) {
            return -1;
        }
        if (bytes_read == 0) {
            return 0;
        }

        byte_count += bytes_read;
        buffer[byte_count] = '\0';
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            return byte_count + 1;
        }
        buffer_tail += bytes_read;
    }

    errno = REQUEST_TOO_LONG;
    return -1;
}
