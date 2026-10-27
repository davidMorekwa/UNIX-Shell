/*
 * strvec.c - Milestone 2: a growable array of strings.
 * Contract: include/strvec.h  (read the growth rule and the failure rules!)
 *
 * Use my_strdup from mystring.h to copy strings.
 */
#include "strvec.h"

#include <stdlib.h>

#include "mystring.h"

void strvec_init(struct strvec *v)
{
    /* TODO */
    (void)v;
}

int strvec_push(struct strvec *v, const char *s)
{
    /* TODO:
     *   1. reject NULL arguments
     *   2. make the copy of s
     *   3. if len == cap, grow: first allocation holds 4 slots, afterwards
     *      double. Assign realloc's result to a TEMPORARY pointer; only if it
     *      is not NULL do you store it in v->items.
     *   4. store the copy and bump len
     * If anything fails, free what you allocated in this call and return -1. */
    (void)v;
    (void)s;
    return -1;
}

const char *strvec_get(const struct strvec *v, size_t i)
{
    /* TODO */
    (void)v;
    (void)i;
    return NULL;
}

char *strvec_pop(struct strvec *v)
{
    /* TODO: the caller becomes the owner of the returned string. */
    (void)v;
    return NULL;
}

void strvec_clear(struct strvec *v)
{
    /* TODO: free the strings, set len to 0, KEEP items and cap. */
    (void)v;
}

void strvec_free(struct strvec *v)
{
    /* TODO: free the strings and the array, then reset to the init state so a
     * second call (or a later push) is safe. */
    (void)v;
}
