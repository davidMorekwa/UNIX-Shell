/*
 * executor.h - running commands with fork / exec / wait / pipe / dup2
 * (Milestone 3)
 *
 * In executor.c you may NOT use system() or popen(): the whole point is to do
 * the fork and exec yourself.
 *
 * Exit-status convention used everywhere below (the same as a real shell):
 *   - the command exited normally          -> its exit code (0..255)
 *   - the command was killed by signal S   -> 128 + S      (SIGINT -> 130)
 *   - exec failed because the program does not exist (ENOENT)      -> 127
 *   - exec failed for another reason (e.g. not executable)         -> 126
 *   - a redirection could not be set up    -> 1
 *
 * This header is the specification for executor.c. Do not change it.
 */
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"
#include "shell_state.h"

/* Set up the command's redirections in the CURRENT process (it is meant to be
 * called in a forked child, just before exec):
 *   - if cmd->infile is not "": open it read-only and make it fd 0 (stdin)
 *   - if cmd->outfile is not "": open it write-only, creating it with mode 0644
 *     if needed, truncating it (append == 0) or in append mode (append != 0),
 *     and make it fd 1 (stdout)
 * (use dup2). The original descriptors returned by open() must be closed, so
 * that NO extra file descriptors stay open afterwards. Careful: if open()
 * happens to return 0 or 1 itself, do not close() it after dup2(fd, fd).
 *
 * Returns 0 on success, -1 on failure with errno set by the failing call.
 * All-or-nothing: on failure fd 0 and fd 1 are unchanged and no descriptor is
 * leaked. The input file is opened FIRST; if that fails the output file must
 * not be created or truncated. A command with no redirections returns 0 and
 * changes nothing. Prints nothing. */
int redirect_io(const struct command *cmd);

/* Run one command and wait for it. In the parent: fork, then wait for exactly
 * that child (retry if waitpid is interrupted by a signal). In the child:
 * restore default signal handling (shell_reset_child_signals), call
 * redirect_io, then execvp the command (command_argv_ptrs gives you the argv).
 *
 * If redirect_io fails the child prints "mysh: redirection failed: <reason>"
 * to stderr and calls _exit(1). If execvp fails the child prints
 * "mysh: <program>: <reason>" to stderr and calls _exit(127) or _exit(126) as
 * described above. Children must use _exit, NOT exit: exit would flush copies
 * of the parent's unflushed stdio buffers a second time.
 *
 * The shell itself (the parent) must not have its own stdin/stdout changed and
 * must not leak descriptors or leave zombie processes behind.
 *
 * Returns the command's status (see convention above), also stored in *st with
 * shell_state_set_status (st may be NULL). Returns -1, changing nothing and
 * forking nothing, if cmd is NULL or has argc == 0; returns -1 if fork fails. */
int execute_command(struct command *cmd, struct shell_state *st);

/* Run   left | right   : two commands connected by a pipe, running AT THE SAME
 * TIME (the left command's stdout becomes the right command's stdin). Create
 * the pipe, fork both children, then wire them up with dup2. In each child:
 * reset signals, dup2 the correct pipe end onto fd 1 (left) or fd 0 (right),
 * CLOSE BOTH pipe ends, then apply that command's own redirections
 * (redirect_io) and exec, with the same error handling as execute_command.
 * The parent must close both pipe ends and wait for both children.
 *
 * (Why close every end everywhere? A reader only sees end-of-file when EVERY
 * copy of the write end is closed. One forgotten copy and the right-hand
 * command waits forever.)
 *
 * Returns the status of the RIGHT-hand command, also stored in *st (st may be
 * NULL). Returns -1 if either command is NULL or has argc == 0, or if pipe or
 * fork fails (clean up whatever was created before the failure). No leaked
 * descriptors and no zombies afterwards. */
int execute_pipe2(struct command *left, struct command *right, struct shell_state *st);

/* For the SHELL process: ignore SIGINT, SIGQUIT and SIGTSTP (Ctrl-C, Ctrl-\
 * and Ctrl-Z must not kill or stop the shell itself). Use sigaction. */
void shell_install_signal_handlers(void);

/* For a CHILD process, right after fork: put SIGINT, SIGQUIT and SIGTSTP back
 * to their default action. This matters because "ignore" is inherited across
 * exec: a child that forgets this cannot be interrupted with Ctrl-C. */
void shell_reset_child_signals(void);

#endif /* EXECUTOR_H */
