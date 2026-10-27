/*
 * iohelp.h - low-level I/O helpers built directly on read() and write()
 * (Milestone 3)
 *
 * In iohelp.c you may NOT use stdio (no FILE*, printf, fgets, ...): only the
 * system calls read and write. A file descriptor is just a small integer.
 *
 * Two facts about the system calls that these helpers exist to handle:
 *   - write(fd, buf, n) may write FEWER than n bytes (a "short write"), for
 *     example when a pipe is full and a signal arrives. You must keep writing
 *     the rest.
 *   - read() and write() can fail with errno == EINTR when a signal handler
 *     interrupts them. That is not an error: just try again.
 *
 * This header is the specification for iohelp.c. Do not change it.
 */
#ifndef IOHELP_H
#define IOHELP_H

#include <stddef.h>
#include <sys/types.h>

/* Write all n bytes of buf to fd. Returns n on success. Returns -1 on a real
 * error (errno is left as set by write). n == 0 returns 0 without calling
 * write. Retries after EINTR and after short writes. */
ssize_t write_all(int fd, const void *buf, size_t n);

/* Copy everything from in_fd to out_fd until end-of-file on in_fd, using a
 * fixed-size buffer (do not try to hold the whole input in memory). Returns the
 * total number of bytes copied, or -1 on error (errno as set by the failing
 * call). Retries reads after EINTR, and uses write_all for writing. Neither
 * descriptor is closed. */
ssize_t copy_fd(int in_fd, int out_fd);

#endif /* IOHELP_H */
