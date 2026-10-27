/*
 * strvec.h - a growable array of strings (Milestone 2)
 *
 * A strvec OWNS its strings: strvec_push stores a private copy of what it is
 * given, and strvec_free releases every copy plus the array. The lexer uses
 * it to hold the tokens of a command line; Week 6's history uses it too.
 *
 * Use your own my_strdup (mystring.h) to make the copies.
 *
 * This header is the specification for strvec.c. Do not change it.
 */
#ifndef STRVEC_H
#define STRVEC_H

#include <stddef.h>

struct strvec {
    char **items;   /* items[0 .. len-1] are malloc'ed strings; NULL while cap == 0 */
    size_t len;     /* number of strings stored                                     */
    size_t cap;     /* number of slots allocated in items                           */
};

/* An empty vector: items NULL, len 0, cap 0. Allocates nothing. */
void strvec_init(struct strvec *v);

/* Append a COPY of s. Returns 0 on success, -1 on failure (s is NULL, or an
 * allocation failed). On failure the vector's contents (len and every stored
 * string) must be exactly as before the call, and nothing may leak.
 *
 * Growth rule (the tests check it): the first push allocates room for 4
 * strings; whenever the array is full, the capacity DOUBLES (4, 8, 16, ...).
 * (On a failed push, only len and the stored strings are promised to be
 * unchanged; the capacity may or may not have grown.)
 * Doubling makes n pushes cost O(n) in total instead of O(n^2).
 * Careful with realloc: if it fails it returns NULL but the old block is still
 * yours. Do not overwrite v->items with the result until you know it worked. */
int strvec_push(struct strvec *v, const char *s);

/* The i-th string, or NULL if i >= len. The vector still owns the string. */
const char *strvec_get(const struct strvec *v, size_t i);

/* Remove the last string and RETURN it; the caller now owns it and must free
 * it. Returns NULL if the vector is empty. Capacity is unchanged. */
char *strvec_pop(struct strvec *v);

/* Free every string and set len to 0, but KEEP the array and cap so the vector
 * can be refilled without reallocating. */
void strvec_clear(struct strvec *v);

/* Free every string and the array itself, then leave *v in the same state as
 * strvec_init. Calling it twice in a row must be harmless. */
void strvec_free(struct strvec *v);

#endif /* STRVEC_H */
