/*
 * shell_state.h - global-ish shell state (Milestone 1)
 *
 * This header is the specification for shell_state.c. Do not change it.
 */
#ifndef SHELL_STATE_H
#define SHELL_STATE_H

#include <stddef.h>

struct shell_state {
    int last_status;   /* exit status of the most recent command, always 0..255 */
    int running;       /* 1 while the main loop should keep going, 0 after "exit" */
};

/* last_status = 0, running = 1. */
void shell_state_init(struct shell_state *st);

/* Store status in st->last_status, reduced to the range 0..255 the way the
 * operating system does it (keep only the low 8 bits):
 *     0 -> 0    7 -> 7    255 -> 255    256 -> 0    257 -> 1    -1 -> 255 */
void shell_state_set_status(struct shell_state *st, int status);

/* Write the prompt into buf (snprintf rules: never write more than size bytes,
 * always '\0'-terminate when size > 0, return the full length the prompt
 * would have had, size may be 0 with buf NULL):
 *     last_status == 0  ->  "mysh> "
 *     otherwise         ->  "[<last_status>] mysh> "      e.g. "[127] mysh> "
 */
int shell_state_format_prompt(const struct shell_state *st, char *buf, size_t size);

#endif /* SHELL_STATE_H */
