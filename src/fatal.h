/*
 * This file is part of Prose Proxy project.
 *
 * Utilities for exiting gracefuly and with proper logging.
 * Contains a list of all exit codes used throughout the project.
 *
 * See fatal.c.
 */

#ifndef FATAL_H
#define FATAL_H

/* Type used where exit status is taken. */
typedef int exit_code;

/* List of exit codes. */
/* Opening server socket failed. */
extern const exit_code FATAL_SOCK_INIT_FAIL;

/* Print out message to stderr, then exit with a given status code. */
void fatal_exit(const char *message, exit_code status);

/* Same as fatal_exit(), but adds errno information to the message.
 * The format of the printed information is as follows:
 * "<message>: <errno message>" */
void fatal_errno(const char *message, exit_code status);

#endif /* FATAL_H */
