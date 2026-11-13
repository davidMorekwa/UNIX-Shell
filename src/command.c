/*
 * command.c - Milestone 1: the shell's data model.
 *
 * The contract for every function is in include/command.h. Read it first.
 * You may use <string.h> and <stdio.h> in this file.
 *
 * Delete each "(void)x;" line as you start using that parameter; those lines
 * only exist so the empty starter code compiles without warnings.
 */
#include "command.h"

#include <stdio.h>
#include <string.h>

void command_init(struct command *cmd)
{
    /* TODO: set every field to its "empty" value (see command.h). */
    (*cmd).argc = 0;
    for(int i = 0; i < MAX_ARGS; i++){
        (*cmd).argv[i][0] = '\0';
    }
    (*cmd).infile[0]='\0';
    (*cmd).outfile[0]='\0';
    (*cmd).append = 0;
}

int command_add_arg(struct command *cmd, const char *arg)
{
    /* TODO: check NULLs, a full command, and an over-long word; otherwise copy
     * arg into cmd->argv[cmd->argc] and bump argc. A failed call must leave
     * *cmd unchanged. */
    if(arg != NULL && cmd != NULL && (*cmd).argc < MAX_ARGS){
        int arg_length = strlen(arg);
        // printf("Arg len: %d\n", arg_length);
        if(arg_length <= MAX_ARG_LEN){
            strncpy((*cmd).argv[(*cmd).argc], arg, arg_length+1);
            (*cmd).argc += 1;
            return 0;

        }
    }
    return -1;
}

int command_set_infile(struct command *cmd, const char *path)
{
    /* TODO: reject NULL, "" and over-long paths; otherwise copy into infile. */
    if(path != NULL && cmd != NULL && *path != '\0'){
        int str_len = strlen(path);
        if(str_len <= MAX_PATH_LEN){
            strcpy((*cmd).infile, path);
            return 0;
        }
    }
    return -1;
}

int command_set_outfile(struct command *cmd, const char *path, int append)
{
    /* TODO: same checks as command_set_infile; also store append as 0 or 1.
     * Careful: a failed call must not change cmd->append either. */
    if(cmd != NULL && path != NULL && *path != '\0'){
        if(append != 0){
            (*cmd).append = 1;
        }
        int path_len = strlen(path);
        if(path_len <= MAX_PATH_LEN){
            strcpy((*cmd).outfile, path);
            return 0;
        }

    }
    return -1;
}

void command_argv_ptrs(struct command *cmd, char *out[MAX_ARGS + 1])
{
    /* TODO: out[i] = cmd->argv[i] for each word, then out[argc] = NULL. */
    if(cmd != NULL){
        for(int i = 0; i < (*cmd).argc; i++){
            out[i] = (*cmd).argv[i];
        }
        out[(*cmd).argc] = NULL;
    }
}

int command_format(const struct command *cmd, char *buf, size_t size)
{
    /* TODO: build "words... < infile > outfile" with snprintf-style truncation
     * and return the FULL length. Hint: write a small helper that appends one
     * piece of text to buf at an offset, always advancing the offset by the
     * piece's full length but only copying what still fits. */
    if (cmd == NULL) return 0;

    int total_len = 0;

    // Handle null termination if size > 0
    if (buf != NULL && size > 0) {
        buf[0] = '\0';
    }

    char *curr_buf = buf;
    size_t rem_size = (buf != NULL && size > 0) ? size : 0;

    // 1. Format commands
    for (int i = 0; i < (*cmd).argc; i++) {
        const char *command_format = (i == 0) ? "%s" : " %s";
        int written = snprintf(curr_buf, rem_size, command_format,(*cmd).argv[i]);
        if (written > 0) {
            total_len += written;
            if ((size_t)written < rem_size) {
                curr_buf += written;
                rem_size -= written;
            } else {
                rem_size = 0;
            }
        }
    }

    // 2. Format infile
    if (cmd->infile[0] != '\0') {
        int written = snprintf(curr_buf, rem_size, " < %s", cmd->infile);
        if (written > 0) {
            total_len += written;
            if ((size_t)written < rem_size) {
                curr_buf += written;
                rem_size -= written;
            } else {
                rem_size = 0;
            }
        }
    }

    // 3. Format outfile
    if (cmd->outfile[0] != '\0') {
        const char *fmt = (cmd->append == 0) ? " > %s" : " >> %s";
        int written = snprintf(curr_buf, rem_size, fmt, cmd->outfile);
        if (written > 0) {
            total_len += written;
            if ((size_t)written < rem_size) {
                curr_buf += written;
                rem_size -= written;
            } else {
                rem_size = 0;
            }
        }
    }

    return total_len;
}

void pipeline_init(struct pipeline *pl)
{
    /* TODO: ncmds = 0, background = 0, and command_init every cmds[i]. */
    for(int i = 0; i < MAX_CMDS; i++){
        (*pl).cmds[i].argc = 0;
        // (*pl).cmds[i].argv[0] = '\0';
        (*pl).cmds[i].infile[0] = '\0';
        (*pl).cmds[i].outfile[0] = '\0';
        (*pl).cmds[i].append = 0;
    }
    (*pl).ncmds = 0;
    (*pl).background = 0;
}
