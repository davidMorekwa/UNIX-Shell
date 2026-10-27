/*
 * joblist.h - a linked list of background jobs (Milestone 2)
 *
 * Nothing runs in the background yet; in Week 5 the shell will add a job here
 * every time it starts a command with '&', and remove it when the process
 * finishes. For now it is "just" a linked list that owns its memory.
 *
 * This header is the specification for joblist.c. Do not change it.
 */
#ifndef JOBLIST_H
#define JOBLIST_H

#include <stddef.h>
#include <sys/types.h>

struct job {
    int id;             /* 1, 2, 3, ... never reused within one list           */
    pid_t pid;
    char *cmdline;      /* malloc'ed copy of the command text                  */
    struct job *next;
};

struct joblist {
    struct job *head;   /* oldest job first                                    */
    int next_id;        /* the id the next successful add will receive         */
};

/* Empty list: head NULL, next_id 1. */
void joblist_init(struct joblist *jl);

/* Append a new job at the END of the list (so the list stays oldest-first) and
 * return its id. A COPY of cmdline is stored (use my_strdup). Returns -1 if
 * cmdline is NULL or an allocation fails; then the list is unchanged, no id is
 * consumed, and nothing leaks (careful: if the node allocation succeeded but
 * the string copy failed, free the node). */
int joblist_add(struct joblist *jl, pid_t pid, const char *cmdline);

/* The job with this pid, or NULL. The list still owns the job. */
struct job *joblist_find_pid(const struct joblist *jl, pid_t pid);

/* Unlink and free the job with this pid (head, middle or tail). Returns 0 if
 * it was found and removed, -1 if there is no such job. Removing a job does
 * not make its id available again. */
int joblist_remove_pid(struct joblist *jl, pid_t pid);

/* Number of jobs in the list. */
int joblist_count(const struct joblist *jl);

/* Write the listing the "jobs" built-in will print, one job per line, oldest
 * first, each line exactly:   [<id>] <pid> <cmdline>\n
 * e.g.   "[1] 4021 sleep 10\n[2] 4030 ls | wc\n"
 * snprintf rules: never write more than size bytes, always '\0'-terminate when
 * size > 0, return the full length the listing would have had (0 for an empty
 * list), size may be 0 with buf NULL. */
int joblist_format(const struct joblist *jl, char *buf, size_t size);

/* Free every job and its cmdline, leaving *jl exactly as joblist_init does
 * (so ids start again at 1). Calling it twice in a row must be harmless. */
void joblist_free(struct joblist *jl);

#endif /* JOBLIST_H */
