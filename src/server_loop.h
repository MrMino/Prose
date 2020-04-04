/*
 * This file is part of Prose Proxy project.
 *
 * Contains only one function - the server_loop().
 *
 * See server_loop.c.
 */

#ifndef SERVER_LOOP_H
#define SERVER_LOOP_H value

/* Serve clients until further notice.
 *
 * This function will exit() if initialization fails.
 */
void server_loop(void);

#endif /* ifndef SERVER_LOOP_H */
