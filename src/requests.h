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

/* Contains host:port components of authority form of request target, as
 * defined in RFC7230 section 5.3.3 (i.e. without userinfo).
 */
typedef struct http_authority {
    char *host;
    int port;
} http_authority;

/* Used to free the memory used by http_authority structure. */
void free_http_authority(http_authority *ptr);

/* Parse authority string in the form of '<host>:<port>'.
 *
 * Assumes that authority_string points to a zero-terminated string that
 * contains a 'host:port' URI. The ':port' component must be present in the
 * string.
 *
 * The 'port' component must contain a number inside the interval 0 < n <
 * 65536.
 *
 * Returns pointer to a newly allocated structure with parsed data or NULL on
 * error. Data in the returned structure is memory-independent from the given
 * string, unlike the data returned by parse_request_line().
 * To free the memory allocated for the returned structure, use
 * free_http_authority().
 */
http_authority *parse_authority(char *authority_string);

#endif /* ifndef REQUESTS_H */
