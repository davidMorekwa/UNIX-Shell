/*
 * parser.h - turning tokens into a struct pipeline (Milestone 2)
 *
 * This header is the specification for parser.c. Do not change it.
 */
#ifndef PARSER_H
#define PARSER_H

#include "command.h"
#include "strvec.h"

#define PARSE_OK                 0
#define PARSE_EMPTY              1    /* no tokens at all: nothing to run          */
#define PARSE_ERR_SYNTAX        (-1)  /* the tokens do not form a valid line       */
#define PARSE_ERR_TOO_MANY_ARGS (-2)  /* more than MAX_ARGS words in one command   */
#define PARSE_ERR_TOO_MANY_CMDS (-3)  /* more than MAX_CMDS commands in a pipeline */
#define PARSE_ERR_TOO_LONG      (-4)  /* a word > MAX_ARG_LEN or a file name
                                         > MAX_PATH_LEN                            */

/* Parse the tokens (as produced by tokenize) into *out. pipeline_init(out) is
 * called first, so *out is fully overwritten whatever it contained. Returns
 * PARSE_OK, PARSE_EMPTY (tokens->len == 0; *out is then an empty pipeline) or a
 * negative PARSE_ERR_* code (the contents of *out are then unspecified).
 *
 * Grammar:
 *     line     :=  (empty)  |  pipeline [ "&" ]
 *     pipeline :=  command ( "|" command )*
 *     command  :=  ( WORD | redirect )+          -- at least one WORD
 *     redirect :=  ( "<" | ">" | ">>" ) WORD
 *
 * Details:
 *  - A WORD is any token that is not one of the operators | < > >> &.
 *    The empty string "" is a valid WORD (but not a valid file name).
 *  - Redirections may appear anywhere inside a command:
 *        "> out ls -l"  and  "ls > out -l"  both give argv {ls, -l}, outfile out.
 *  - Every command needs at least one WORD: "| ls", "ls |", "ls | | wc" and
 *    "> out" are syntax errors.
 *  - A redirect operator must be followed by a WORD (not an operator, not the
 *    end of the line), and that WORD must not be "": otherwise syntax error.
 *  - "<" is only allowed in the FIRST command, ">" and ">>" only in the LAST
 *    command of a pipeline (a middle command's input/output belongs to the
 *    pipe). Anything else is a syntax error.
 *  - Each of "<" and ">"/">>" may appear at most once per command; a second
 *    one is a syntax error.
 *  - "&" is only allowed as the very LAST token, after a complete pipeline.
 *    It sets out->background. "&" alone, "ls & ls" and "ls &&" are errors.
 *  - Limits: a command with more than MAX_ARGS words gives
 *    PARSE_ERR_TOO_MANY_ARGS; more than MAX_CMDS commands gives
 *    PARSE_ERR_TOO_MANY_CMDS; a word longer than MAX_ARG_LEN or a file name
 *    longer than MAX_PATH_LEN gives PARSE_ERR_TOO_LONG.
 *
 * Use your Milestone 1 functions (command_add_arg, command_set_infile, ...) and
 * my_strcmp to do the work. */
int parse_pipeline(const struct strvec *tokens, struct pipeline *out);

#endif /* PARSER_H */
