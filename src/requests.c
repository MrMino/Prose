/*
 * This file is part of Prose Proxy project.
 *
 * See requests.h.
 */

#include <stdlib.h>
#include <string.h>

#include "requests.h"

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
