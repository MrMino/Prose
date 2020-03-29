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

/* As defined in RFC7230 section 3.1.1.*/
typedef struct http_request_line {
    char *method;
    char *uri;
    char *http_version;
} http_request_line;

void free_http_request_line(http_request_line *ptr) {
    /* ptr->method should be the same thing as the char array allocated in
     * parse_request_line(). Free it to free all the other fields.
     */
    free(ptr->method);
    free(ptr);
}

/* Parse data from buffer as HTTP request.
 *
 * Assumes that request_line points to a zero-terminated string with the first
 * line of the HTTP request.
 *
 * Returns pointer to a newly allocated structure with parsed data or NULL on
 * error.
 * The members of the created structure point to different places in
 * request_line string, and as such the string should not be free()d.
 * Instead, to free the allocated memory, use free_http_request_line().
 *
 * NOTE: this function uses strtok() which is NOT thread-safe.
 */
http_request_line *parse_request_line(char *request_line) {
    http_request_line *parsed_line = malloc(sizeof(http_request_line));
    if (parsed_line == NULL)
        return NULL;

    parsed_line->method = strtok(request_line, " ");
    parsed_line->uri = strtok(NULL, " ");
    parsed_line->http_version = strtok(NULL, " ");

    if (parsed_line->method == NULL || parsed_line->uri == NULL
        || parsed_line->http_version == NULL) {
        free_http_request_line(parsed_line);
        return NULL;
    }
    return parsed_line;
}

/* Copy characters in the string up to the first CRLF sequence.
 *
 * Returns pointer to a copied string or NULL on error. The new string contains
 * the CRLF character and is zero-terminated.
 *
 * If CRLF is not present in the given string, returns NULL.
 */
char *copy_line_crlf(const char *string) {
    char *line_end = strstr(string, "\r\n");
    if (line_end == NULL)
        return NULL;
    int line_size = line_end - string;

    char *line = malloc(sizeof(char) * line_size + 1);
    if (line == NULL)
        return NULL;
    strncpy(line, string, line_size);
    line[line_size] = '\0';
    return line;
}

/* Read at most bytes_max data available in the socket.
 *
 * This is a simple wrapper around select() -> recv().
 * Received data is put into memory address pointed to by buffer argument.
 * Data IS NOT null-terminated.
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
 *  - received bytes_max - 1 bytes before double CRLF occured
 *  - timeout is exceeded
 *
 * Received data is put into memory address pointed to by buffer argument.
 * Data in the buffer is null terminated.
 *
 * NOTE: if additional bytes after double CRLF are received, they are put into
 * the buffer with the rest of the data.
 *
 * Returns number of bytes received incremented by one (for the zero byte at
 * the end of the data), if double CRLF is in the buffer after reading. Returns
 * -1 on an error.
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

/* TODO
 * close() should be called on a SIGINT.
 * Buffer length should be 8KiB, maybe it should be circular?
 * After first request, use splice() loop.
 */

void serve_client(int client_socketfd) {
    char *buffer = malloc(BUFFER_LEN);

    struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
    int received_bytes = recv_request_head(client_socketfd, buffer,
                                           BUFFER_LEN - 1, &timeout);
    if (received_bytes > 0) {
        buffer[received_bytes] = '\0';

        char *method_line = copy_line_crlf(buffer);
        if (method_line == NULL) {
            if (errno == 0) {
                puts("BAD REQUEST LINE");
            } else {
                puts("ERROR PARSING REQUEST LINE");
            }
            free(buffer);
            close(client_socketfd);
            return;
        }

        http_request_line *request = parse_request_line(method_line);
        if (request == NULL) {
            if (errno == 0) {
                puts("BAD REQUEST LINE");
            } else {
                puts("ERROR PARSING REQUEST LINE");
            }
            free(buffer);
            close(client_socketfd);
            return;
        }

        printf("Method: %s\nURI: %s\nVersion: %s\n", request->method,
               request->uri, request->http_version);
        free_http_request_line(request);
    } else {
        printf("TIMED OUT\n");
    }

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
