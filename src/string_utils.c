/*
 * This file is part of Prose Proxy project.
 *
 * See string_utils.h
 */

#include <stdlib.h>
#include <string.h>

#include "string_utils.h"

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
