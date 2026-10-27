/*
 * parser.c - Milestone 2: tokens -> struct pipeline.
 * Contract: include/parser.h  (the grammar and every rule are there)
 *
 * Suggested design: call pipeline_init(out); keep a pointer `cur` to the
 * command being filled in (starting with out->cmds[0], out->ncmds = 1); then
 * loop over the tokens and decide for each one:
 *     "|"            -> validate the command you just finished, move `cur` on
 *     "<", ">", ">>" -> the NEXT token is the file name: validate, store, skip it
 *     "&"            -> must be the last token
 *     anything else  -> a word: command_add_arg
 * After the loop, check that the last command has at least one word.
 *
 * Write small helpers such as  is_operator(const char *)  to keep the main
 * function readable, and use my_strcmp (not ==) to compare strings.
 */
#include "parser.h"

#include "mystring.h"

int parse_pipeline(const struct strvec *tokens, struct pipeline *out)
{
    /* TODO */
    (void)tokens;
    (void)out;
    return PARSE_ERR_SYNTAX;
}
