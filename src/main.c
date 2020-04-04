/*
 * This file is part of Prose Proxy project.
 */

#include "server_loop.h"

/* TODO
 * close() should be called on a SIGINT.
 * After first request, use splice() loop.
 */

int main() { server_loop(); }
