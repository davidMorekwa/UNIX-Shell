/*
 * builtin.h - names of the shell's built-in commands (Milestone 1)
 *
 * Built-ins must run inside the shell process itself (a forked child could
 * not change the shell's directory, for example). In Milestone 1 you only
 * need to RECOGNISE them; later milestones will implement them.
 *
 * This header is the specification for builtin.c. Do not change it.
 */
#ifndef BUILTIN_H
#define BUILTIN_H

enum builtin_id {
    BUILTIN_NONE = 0,   /* not a built-in */
    BUILTIN_EXIT,       /* "exit"    */
    BUILTIN_CD,         /* "cd"      */
    BUILTIN_PWD,        /* "pwd"     */
    BUILTIN_HELP,       /* "help"    */
    BUILTIN_JOBS,       /* "jobs"    */
    BUILTIN_HISTORY     /* "history" */
};

/* Return the id for an exact, case-sensitive match of name, or BUILTIN_NONE
 * if name is not a built-in (including "", names with extra spaces such as
 * "exit ", and NULL). Hint: a small static table of struct { name; id } and a
 * loop is cleaner than a long if/else chain. */
enum builtin_id builtin_lookup(const char *name);

/* The inverse: the name for id ("exit" for BUILTIN_EXIT, ...), or NULL for
 * BUILTIN_NONE and for any value that is not a valid id. */
const char *builtin_name(enum builtin_id id);

#endif /* BUILTIN_H */
