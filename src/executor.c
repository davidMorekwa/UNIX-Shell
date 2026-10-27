/*
 * executor.c - Milestone 3: fork / exec / wait, redirection, pipes, signals.
 * Contract: include/executor.h  (read every paragraph: the details matter)
 *
 * RULES (checked by "make lint"): do not use the library functions that run a
 * shell command for you (the two named in executor.h). Do the fork and exec
 * yourself, and avoid writing those two names followed by an opening
 * parenthesis, even in comments: the check is a plain text search.
 *
 * Suggested structure (write these as static helpers):
 *
 *   static int  decode_status(int wstatus);   // exited -> code; signaled -> 128 + sig
 *   static int  wait_for(pid_t pid);          // waitpid, retrying on EINTR
 *   static void run_child(struct command *c); // what a child does after fork:
 *                                             //   reset signals, redirect_io, execvp,
 *                                             //   report errors, _exit; never returns
 *
 * execute_command and execute_pipe2 then become short: they fork, call
 * run_child in the child(ren), and wait in the parent.
 */
#include "executor.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void shell_install_signal_handlers(void)
{
    /* TODO: use sigaction (not signal) to set SIGINT, SIGQUIT and SIGTSTP to
     * SIG_IGN. A small helper  set_disposition(int sig, void (*handler)(int))
     * lets you reuse the code in shell_reset_child_signals. */
}

void shell_reset_child_signals(void)
{
    /* TODO: the same three signals back to SIG_DFL. */
}

int redirect_io(const struct command *cmd)
{
    /* TODO:
     *   1. if infile is set: open it O_RDONLY (remember the descriptor)
     *      if that fails: return -1 (do not touch the output file!)
     *   2. if outfile is set: open it O_WRONLY | O_CREAT and O_APPEND or
     *      O_TRUNC, mode 0644. If that fails: close the input descriptor you
     *      opened in step 1 (but keep errno from the failed open) and return -1
     *   3. dup2 each descriptor onto 0 / 1, then close the original.
     *      Special case: if open() returned 0 or 1 itself, dup2 does nothing
     *      and you must NOT close it. */
    (void)cmd;
    return -1;
}

int execute_command(struct command *cmd, struct shell_state *st)
{
    /* TODO: reject NULL/empty commands; fork; the child runs run_child(cmd);
     * the parent waits, decodes the status, stores it in *st (if st != NULL)
     * and returns it. */
    (void)cmd;
    (void)st;
    return -1;
}

int execute_pipe2(struct command *left, struct command *right, struct shell_state *st)
{
    /* TODO: pipe(fds); fork the left child (dup2 fds[1] onto STDOUT_FILENO,
     * close BOTH ends, run_child); fork the right child (dup2 fds[0] onto
     * STDIN_FILENO, close both ends, run_child); the parent closes both ends
     * and waits for both. Return the right-hand status. If a fork fails, close
     * the pipe and wait for any child already started. */
    (void)left;
    (void)right;
    (void)st;
    return -1;
}
