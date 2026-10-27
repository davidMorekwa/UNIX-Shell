/*
 * mystring.c - Milestone 2: your own <string.h> functions.
 * Contract: include/mystring.h  (behave EXACTLY like the libc versions)
 *
 * RULES (checked by "make lint"):
 *   - do not include <string.h>
 *   - do not call any libc string or memory function from this file
 *   - <stdlib.h> is fine, for malloc
 * Also avoid writing the libc function names followed by an opening
 * parenthesis, even inside comments: the lint check is a plain text search.
 *
 * Hint: work with pointers, not indexes, in at least a couple of these. Try
 * writing my_strlen with a pointer that walks to the terminating '\0'.
 */
#include "mystring.h"

#include <stdlib.h>

size_t my_strlen(const char *s)
{
    /* TODO */
    (void)s;
    return 0;
}

int my_strcmp(const char *a, const char *b)
{
    /* TODO: compare as UNSIGNED char (cast before subtracting). */
    (void)a;
    (void)b;
    return 0;
}

int my_strncmp(const char *a, const char *b, size_t n)
{
    /* TODO: like my_strcmp, but give up after n characters. */
    (void)a;
    (void)b;
    (void)n;
    return 0;
}

char *my_strcpy(char *dst, const char *src)
{
    /* TODO */
    (void)dst;
    (void)src;
    return NULL;
}

char *my_strncpy(char *dst, const char *src, size_t n)
{
    /* TODO: copy up to n chars; if src ends first, pad dst with '\0' up to n
     * bytes. Do not write more than n bytes. */
    (void)dst;
    (void)src;
    (void)n;
    return NULL;
}

char *my_strchr(const char *s, int c)
{
    /* TODO: remember that c is converted to char, and that the terminating
     * '\0' counts as part of the string. */
    (void)s;
    (void)c;
    return NULL;
}

char *my_strdup(const char *s)
{
    /* TODO: NULL in -> NULL out; malloc(length + 1); check the malloc result
     * BEFORE copying; use your own my_strlen and my_strcpy. */
    (void)s;
    return NULL;
}
