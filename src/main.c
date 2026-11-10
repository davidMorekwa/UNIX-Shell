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
    struct shell_state st;
    char line[1024];
    char prompt[64];

    shell_state_init(&st);

    while (st.running) {
        shell_state_format_prompt(&st, prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);

        int result = read_line(stdin, line, sizeof(line));
        if (result == READ_EOF) {
            putchar('\n');
            break;
        }
        if (result == READ_TOO_LONG) {
            fprintf(stderr, "mysh: line too long\n");
            shell_state_set_status(&st, 1);
            continue;
        }

        char *command = str_trim(line);
        if (command[0] == '\0') {
            continue;
        }
        if (builtin_lookup(command) == BUILTIN_EXIT) {
            st.running = 0;
            continue;
        }

        printf("You said: %s\n", command);
        shell_state_set_status(&st, 0);
    }

    return 0;
}
