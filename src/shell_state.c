/*
 * shell_state.c - Milestone 1: the shell's state and its prompt.
 * Contract: include/shell_state.h
 */
#include "shell_state.h"

#include <stdio.h>

void shell_state_init(struct shell_state *st)
{
    /* TODO */
    if(st != NULL){
        (*st).last_status = 0;
        (*st).running = 1;
    }
}

void shell_state_set_status(struct shell_state *st, int status)
{
    /* TODO: keep only the low 8 bits. Think about what & does to a negative
     * number, and test -1. */
    if(st != NULL){
        (*st).last_status = status & 0xFF;
    }

}

int shell_state_format_prompt(const struct shell_state *st, char *buf, size_t size)
{
    /* TODO: snprintf already has exactly the truncation behaviour the header
     * asks for: use its return value. */
    if(st == NULL) return 0;
    int total_length = 0;
    if(buf != NULL && size > 0){
        buf[0] = '\0';
    }
    char *curr_buf = buf;
    size_t rem_size = (buf != NULL && size > 0) ? size : 0;
    char *prompt_format = ((*st).last_status == 0) ? "mysh> " : "[%d] mysh> ";
    int bytes_written = snprintf(curr_buf, rem_size, prompt_format, (*st).last_status);
    if(bytes_written > 0){
        total_length += bytes_written;
        if((size_t)bytes_written < rem_size){
            curr_buf += bytes_written;
            rem_size -= bytes_written;
        } else {
            rem_size = 0; // truncation happend
        }
    }
    return total_length;

    // (void)st;
    // (void)buf;
    // (void)size;
    // return 0;
}
