/*
 * joblist.c - Milestone 2: a linked list of background jobs.
 * Contract: include/joblist.h
 *
 * Hint for add and remove: a "pointer to pointer" (struct job **link) that
 * points at either jl->head or some node's `next` field lets you handle the
 * first node, a middle node and the last node with the SAME code, with no
 * special case for the head.
 */
#include "joblist.h"

#include <stdio.h>
#include <stdlib.h>

#include "mystring.h"

void joblist_init(struct joblist *jl)
{
    /* TODO */
    (void)jl;
}

int joblist_add(struct joblist *jl, pid_t pid, const char *cmdline)
{
    /* TODO: malloc the node, my_strdup the text; if the second step fails, free
     * the node. Append at the END of the list. Only bump next_id on success. */
    (void)jl;
    (void)pid;
    (void)cmdline;
    return -1;
}

struct job *joblist_find_pid(const struct joblist *jl, pid_t pid)
{
    /* TODO */
    (void)jl;
    (void)pid;
    return NULL;
}

int joblist_remove_pid(struct joblist *jl, pid_t pid)
{
    /* TODO: unlink the node FIRST, then free its cmdline and the node itself. */
    (void)jl;
    (void)pid;
    return -1;
}

int joblist_count(const struct joblist *jl)
{
    /* TODO */
    (void)jl;
    return 0;
}

int joblist_format(const struct joblist *jl, char *buf, size_t size)
{
    /* TODO: one snprintf per job, appended after the previous output, counting
     * the FULL length of every line even once the buffer is full. Take care
     * that size may be 0 and that an empty list must still give "". */
    (void)jl;
    (void)buf;
    (void)size;
    return 0;
}

void joblist_free(struct joblist *jl)
{
    /* TODO: read `next` BEFORE freeing a node. Finish with joblist_init. */
    (void)jl;
}
