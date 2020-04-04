/*
 * This file is part of Prose Proxy project.
 *
 * Functions that parse HTTP requests (including URLs).
 */

#ifndef REQUESTS_H
#define REQUESTS_H

/* Holds information about the first line of HTTP Request.
 * Should only be created by parse_request_line().
 *
 * As defined in RFC7230 section 3.1.1.
 */
typedef struct http_request_line {
    char *method;
    char *uri;
    char *http_version;
} http_request_line;

/* Used to free the memory used by http_request_line structure.  */
void free_http_request_line(http_request_line *ptr);

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
http_request_line *parse_request_line(char *request_line);

#endif /* ifndef REQUESTS_H */
