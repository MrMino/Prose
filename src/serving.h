/*
 * This file is part of Prose Proxy project.
 *
 * Functions that handle serving a single client.
 *
 * See serving.c.
 */

#ifndef SERVING_H
#define SERVING_H

#define BUFFER_LEN 1024 * 8

/* Serve a single incomming connection.
 *
 * The meat and potatoes of the proxy begin in this function.
 *
 * Takes a file descriptor of the accept()-ed socket, reads the client request,
 * and passes it to the correct remote host. Or crashes with an error -
 * depending on what was in the request.
 *
 * This fork()-s upon a call, and performs cleanup in the parent side, i.e.
 * close()-s the client socket (as parent processes should).
 */
void serve_client(int client_socketfd);

#endif /* ifndef SERVING_H */
