#ifndef BUFFERCONFIG_HPP
#define BUFFERCONFIG_HPP

/*
** BUFFER_SIZE configuration
** This can be overridden at compile time using -D flag in Makefile
** Example: make BUFFER_SIZE=16384
** Default: 8192 bytes (8KB)
*/
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 8192
#endif

/*
** Unified buffer size for all read/write operations
** Used in ClientSocket, StreamSocket, and other I/O operations
*/
#define READ_BUFFER_SIZE BUFFER_SIZE
#define WRITE_BUFFER_SIZE BUFFER_SIZE

#endif

