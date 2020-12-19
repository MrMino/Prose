/*
 * This file is part of Prose Proxy project.
 *
 * Functions that create connections based on different types of given data.
 *
 * See connecting.c.
 */

#ifndef CONNECTING_H
#define CONNECTING_H

#include "requests.h"

/* Issue a connection using given 'host:port'.
 *
 * Such connections are used within the context of HTTP CONNECT requests.
 *
 * The authority argument must be a pointer to a structure created by
 * parse_authority().
 *
 * On success, returns the file descriptor of a newly created and data-ready
 * socket. On error, -1 is returned, and errno is set approprately.
 */
int connect_to_authority(http_authority *authority);

#endif /* ifndef CONNECTING_H */
