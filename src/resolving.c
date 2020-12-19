/*
 * This file is part of Prose Proxy project.
 *
 * See resolving.h.
 */

#define _POSIX_C_SOUIRCE 201112L

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "requests.h"

#include "connecting.h"

struct addrinfo default_hints = {
        /* Give addresses for any socket family */
        .ai_family = AF_UNSPEC,
        /* Give addresses for TCP */
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
        /* Don't give v4/v6 addresses if localhost has no interfaces for
           either. */
        .ai_flags = AI_ADDRCONFIG,

        .ai_canonname = NULL,
        .ai_addrlen = 0,
        .ai_addr = NULL,
        .ai_next = NULL,
};

addrinfo *resolve_authority(http_authority *authority) {
    int gai_failure = getaddrinfo(authority.host
}
