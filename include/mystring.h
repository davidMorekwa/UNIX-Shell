/*
 * mystring.h - your own versions of a few <string.h> functions (Milestone 2)
 *
 * Each function must behave EXACTLY like its libc namesake (the tests compare
 * against libc). In mystring.c you may not include <string.h> or call any libc
 * string/memory function: the point is to practise pointers and arrays.
 * (malloc is allowed, for my_strdup.)
 *
 * This header is the specification for mystring.c. Do not change it.
 */
#ifndef MYSTRING_H
#define MYSTRING_H

#include <stddef.h>

/* Number of characters before the terminating '\0'. */
size_t my_strlen(const char *s);

/* Compare a and b character by character, treating each as an UNSIGNED char.
 * Returns <0, 0 or >0 as a is less than, equal to, or greater than b. Only the
 * sign matters. */
int my_strcmp(const char *a, const char *b);

/* Like my_strcmp but looks at no more than n characters. n == 0 returns 0. */
int my_strncmp(const char *a, const char *b, size_t n);

/* Copy src, including its '\0', to dst. Returns dst. */
char *my_strcpy(char *dst, const char *src);

/* Copy at most n characters of src to dst. If src is shorter than n, the rest
 * of dst (up to n bytes in total) is filled with '\0'. If src has n or more
 * characters, dst is NOT '\0'-terminated. Writes exactly n bytes, never more.
 * Returns dst. */
char *my_strncpy(char *dst, const char *src, size_t n);

/* Pointer to the first occurrence of c (converted to char) in s, or NULL.
 * The terminating '\0' counts as part of the string: my_strchr(s, '\0')
 * returns a pointer to it. */
char *my_strchr(const char *s, int c);

/* Return a freshly malloc'ed copy of s (the caller frees it). Returns NULL if
 * s is NULL or if malloc fails. */
char *my_strdup(const char *s);

#endif /* MYSTRING_H */
