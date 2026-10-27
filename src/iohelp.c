/*
 * iohelp.c - Milestone 3: low-level I/O on file descriptors.
 * Contract: include/iohelp.h
 *
 * RULES (checked by "make lint"): no stdio here. Do not include <stdio.h> and
 * do not call fopen, fread, fwrite, fgets, printf, fprintf, puts, fflush, ...
 * Only the system calls read and write. Avoid writing those names followed by
 * an opening parenthesis, even in comments: the check is a plain text search.
 */
#include "iohelp.h"

#include <errno.h>
#include <unistd.h>

ssize_t write_all(int fd, const void *buf, size_t n)
{
    /* TODO: loop until all n bytes are written.
     *   - write() may accept fewer bytes than you asked: advance and go again
     *   - if write() fails with errno == EINTR, just try again
     *   - any other failure: return -1 */
    (void)fd;
    (void)buf;
    (void)n;
    return -1;
}

ssize_t copy_fd(int in_fd, int out_fd)
{
    /* TODO: read() into a fixed-size buffer (4096 bytes is a fine size) until
     * it returns 0 (end of file); hand each chunk to write_all. Retry reads
     * that fail with EINTR. Return the total, or -1 on error. */
    (void)in_fd;
    (void)out_fd;
    return -1;
}
