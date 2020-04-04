/*
 * This file is part of Prose Proxy project.
 *
 * Functions that move data around sockets.
 *
 * See transport.c
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <errno.h>

#define REQUEST_TIMED_OUT EWOULDBLOCK
#define REQUEST_TOO_LONG ENOMEM

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
int timed_read(int sockfd, char *buffer, int bytes_max,
               struct timeval *timeout);

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
int recv_request_head(int sockfd, char *buffer, int bytes_max,
                      struct timeval *timeout);

#endif /* ifndef TRANSPORT_H */
