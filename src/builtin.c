/*
 * builtin.c - Milestone 1: recognising the built-in command names.
 * Contract: include/builtin.h
 */
#include "builtin.h"

#include <stddef.h>
#include <string.h>

/* TODO: define a struct { const char *name; enum builtin_id id; } and a static
 * table with one entry per built-in. Both functions below can then be a simple
 * loop over the table. */

enum builtin_id builtin_lookup(const char *name)
{
    /* TODO */
    (void)name;
    return BUILTIN_NONE;
}

const char *builtin_name(enum builtin_id id)
{
    /* TODO */
    (void)id;
    return NULL;
}
