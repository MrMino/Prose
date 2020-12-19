/*
 * This file is part of Prose Proxy project.
 *
 * See requests.h.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "requests.h"

#define BASE_TEN 10
#define MIN_PORT 1
#define MAX_PORT 65535

void free_http_request_line(http_request_line *ptr) {
    /* ptr->method should be the same thing as the char array allocated in
     * parse_request_line(). Free it to free all the other fields. */
    free(ptr->method);
    free(ptr);
}

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

void free_http_authority(http_authority *ptr) {
    free(ptr->host);
    free(ptr->port);
    free(ptr);
}

http_authority *parse_authority(char *authority_string) {
    char *delimiter_pos = strchr(authority_string, ':');
    if (delimiter_pos == NULL)
        return NULL;

    int host_len = delimiter_pos - authority_string;
    if (host_len == 0)
        return NULL;

    char *port_string = authority_string + host_len + 1;

    int port_len = strlen(port_string);
    if (port_len == 0 || port_len > 5)
        return NULL;

    /* Required, since strtol ignores first few whitespace characters. */
    if (isspace(port_string[0]))
        return NULL;

    char *first_nonnumeric;
    int port = strtol(port_string, &first_nonnumeric, BASE_TEN);
    if (*first_nonnumeric != '\0')
        return NULL;

    if (!(MIN_PORT <= port && port <= MAX_PORT))
        return NULL;

    http_authority *parsed_data = malloc(sizeof(http_authority));
    if (parsed_data == NULL)
        return NULL;

    parsed_data->host = malloc(host_len + 1);
    strncpy(parsed_data->host, authority_string, host_len);
    parsed_data->host[host_len + 1] = '\0';

    parsed_data->port = port;

    return parsed_data;
}

/* Contains a mapping of method name (HTTP verb) to a callback that is invoked
 * to act upon a request with that method name.
 */
struct method_callback {
    /* The method for which operation is called. */
    const char *verb;
    /* Operation to call as a logic for a given method.
     * Takes two arguments: pointer to the current buffer of the request and
     * a file descriptor of the socket to which the client is connected.
     */
    int (*operation)(const char *, int);
};

struct method_callback accepted_methods[] = {
        {.verb = NULL, .operation = NULL}};
