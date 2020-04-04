/*
 * This file is part of Prose Proxy project.
 *
 * Utilities for operating on char arrays.
 *
 * See string_utils.c
 */

#ifndef STRING_UTILS_H

/* Copy characters in the string up to the first CRLF sequence.
 *
 * Returns pointer to a copied string or NULL on error. The new string contains
 * the CRLF character and is null-terminated.
 *
 * If CRLF is not present in the given string, returns NULL.
 */
char *copy_line_crlf(const char *string);

#define STRING_UTILS_H
#endif /* ifndef STRING_UTILS_H */
