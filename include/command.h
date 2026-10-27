/*
 * command.h - the shell's data model (Milestone 1)
 *
 * A line like     cat < in.txt | sort > out.txt &
 * is represented as ONE struct pipeline holding TWO struct commands.
 * Everything here uses fixed-size arrays: no dynamic memory yet.
 *
 * This header is the specification for command.c. Do not change it.
 */
#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>

#define MAX_ARGS      16   /* max words in one command (including the program name) */
#define MAX_ARG_LEN   64   /* max length of one word, NOT counting the '\0'         */
#define MAX_PATH_LEN 128   /* max length of a redirection file name, NOT counting '\0' */
#define MAX_CMDS       8   /* max commands in one pipeline                          */

struct command {
    int  argc;                           /* number of words stored in argv        */
    char argv[MAX_ARGS][MAX_ARG_LEN + 1];/* argv[0] is the program name           */
    char infile[MAX_PATH_LEN + 1];       /* "<  file": "" means no redirection    */
    char outfile[MAX_PATH_LEN + 1];      /* ">  file" or ">> file": "" means none */
    int  append;                         /* 1 for ">>", 0 for ">" (only meaningful
                                            when outfile is not "")               */
};

struct pipeline {
    struct command cmds[MAX_CMDS];       /* cmds[0 .. ncmds-1] are in use         */
    int ncmds;
    int background;                      /* 1 if the line ended with '&'          */
};

/* Set every field of *cmd to its "empty" value: argc 0, every argv[i] is "",
 * infile and outfile are "", append is 0. */
void command_init(struct command *cmd);

/*
 * Append a copy of arg as the next word.
 * Returns 0 on success. Returns -1 and leaves *cmd completely unchanged if
 *   - cmd or arg is NULL,
 *   - the command already holds MAX_ARGS words, or
 *   - arg is longer than MAX_ARG_LEN characters.
 * The empty string "" IS a valid word. */
int command_add_arg(struct command *cmd, const char *arg);

/* Set the "<" redirection. Returns 0 on success.
 * Returns -1 and leaves *cmd unchanged if cmd or path is NULL, path is empty,
 * or path is longer than MAX_PATH_LEN characters. */
int command_set_infile(struct command *cmd, const char *path);

/* Set the ">" (append == 0) or ">>" (append != 0) redirection. Stores 1 in
 * cmd->append for any nonzero value. Same failure rules (and the same "leave
 * everything unchanged" promise, including cmd->append) as command_set_infile. */
int command_set_outfile(struct command *cmd, const char *path, int append);

/* Fill out[0 .. argc-1] with pointers to cmd->argv[0 .. argc-1] and set
 * out[argc] = NULL, producing the NULL-terminated argument vector that
 * execvp() wants. The pointers point INTO *cmd (nothing is copied). */
void command_argv_ptrs(struct command *cmd, char *out[MAX_ARGS + 1]);

/* Write a one-line, human-readable form of the command into buf, e.g.
 *     "ls -l"      "sort < in.txt"      "cat < a > b"      "echo hi >> log"
 * Pieces are separated by single spaces, in this order: the words, then
 * "< infile" if set, then "> outfile" or ">> outfile" if set. No quoting is
 * added. An empty command produces "".
 *
 * Behaves like snprintf: never writes more than size bytes (including the
 * '\0'), always '\0'-terminates when size > 0, and RETURNS THE LENGTH THE
 * FULL TEXT WOULD HAVE HAD (excluding the '\0'), even if it was truncated.
 * size may be 0, in which case buf may be NULL and nothing is written. */
int command_format(const struct command *cmd, char *buf, size_t size);

/* Initialise a pipeline: ncmds 0, background 0, and every cmds[i] empty
 * (as by command_init). */
void pipeline_init(struct pipeline *pl);

#endif /* COMMAND_H */
