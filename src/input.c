/*
 * input.c - Milestone 1: reading one line of input, and trimming it.
 * Contract: include/input.h
 */
#include "input.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

int read_line(FILE *in, char *buf, size_t size)
{
    /* TODO: fgets() into buf. If it returns NULL -> READ_EOF.
     * If the line ended with '\n', remove it and return the length.
     * If not, work out WHY there is no '\n': the line exactly filled the buffer,
     * it was the last line of the file, or it was too long (then discard the
     * rest of it so the next call starts on the next line). */
    (void)in;
    (void)buf;
    (void)size;
    return READ_EOF;
}

char *str_trim(char *s)
{
    /* TODO: skip leading whitespace (isspace needs an unsigned char!), then
     * walk back from the end to cut trailing whitespace. */
    (void)s;
    return NULL;
}
