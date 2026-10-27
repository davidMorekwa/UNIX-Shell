/*
 * shell_state.c - Milestone 1: the shell's state and its prompt.
 * Contract: include/shell_state.h
 */
#include "shell_state.h"

#include <stdio.h>

void shell_state_init(struct shell_state *st)
{
    /* TODO */
    (void)st;
}

void shell_state_set_status(struct shell_state *st, int status)
{
    /* TODO: keep only the low 8 bits. Think about what & does to a negative
     * number, and test -1. */
    (void)st;
    (void)status;
}

int shell_state_format_prompt(const struct shell_state *st, char *buf, size_t size)
{
    /* TODO: snprintf already has exactly the truncation behaviour the header
     * asks for: use its return value. */
    (void)st;
    (void)buf;
    (void)size;
    return 0;
}
