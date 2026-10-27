/*
 * main.c - the shell's main loop (Milestone 1).
 *
 * Behaviour required for Milestone 1 (tests/integration_m1.sh checks it):
 *
 *   1. Build the prompt with shell_state_format_prompt, print it, and fflush
 *      stdout (the prompt has no newline, so it would otherwise stay buffered).
 *   2. Read a line with read_line.
 *        READ_EOF      -> print a single "\n" and leave the loop (Ctrl-D).
 *        READ_TOO_LONG -> print "mysh: line too long\n" to STDERR, set the
 *                         status to 1, and go around the loop again.
 *   3. Trim the line with str_trim. A blank line: just prompt again.
 *   4. If the line is the built-in "exit", clear st.running and stop.
 *   5. Otherwise print "You said: <line>\n" and set the status to 0.
 *
 * Later milestones replace step 5 with real parsing and execution, so keep the
 * structure of the loop clean.
 */
#include <stdio.h>

#include "builtin.h"
#include "input.h"
#include "shell_state.h"

int main(void)
{
    /* TODO: declare a struct shell_state, a line buffer (1024 bytes is fine)
     * and a prompt buffer, then write the loop described above. */
    return 0;
}
