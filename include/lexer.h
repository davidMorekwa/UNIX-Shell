/*
 * lexer.h - splitting a command line into tokens (Milestone 2)
 *
 * This is the tokenizer the real shell calls on every line you type. You must
 * write the splitting yourself (no strtok, strsep or sscanf).
 *
 * This header is the specification for lexer.c. Do not change it.
 */
#ifndef LEXER_H
#define LEXER_H

#include "strvec.h"

#define LEX_ERR_QUOTE (-1)   /* a ' or " was never closed        */
#define LEX_ERR_NOMEM (-2)   /* an allocation failed             */

/* Split line into tokens and append them to *out (which must be initialised
 * and empty). Returns the number of tokens (0 for an empty or all-blank line),
 * or a negative LEX_ERR_* code. On ANY error, *out is left empty (len == 0) and
 * nothing leaks.
 *
 * Rules:
 *  1. Whitespace (space, tab, newline...) separates tokens and is otherwise
 *     ignored. Leading, trailing and repeated whitespace produce no empty
 *     tokens.
 *  2. These operators are ALWAYS tokens of their own, even with no spaces
 *     around them:    |    <    >    >>    &
 *     ">>" is one token (so ">>>" is ">>" followed by ">").
 *     Every "&" is a token of its own ("&&" is two tokens "&" "&").
 *         "ls>out"   -> ls  >  out
 *         "a|b"      -> a  |  b
 *         "sleep 5&" -> sleep  5  &
 *  3. Quotes group characters. Inside '...' or "..." whitespace and operator
 *     characters are ordinary text. The quote characters themselves are
 *     removed. Text next to a quoted part stays in the SAME token:
 *         "hello world"          -> hello world
 *         he"llo wo"rld          -> hello world       (one token)
 *         ""                     -> an EMPTY token (still a token!)
 *         "it's"  and  'say "hi"'-> it's   and   say "hi"
 *     There are no escape sequences: a backslash is an ordinary character.
 *  4. An unterminated quote returns LEX_ERR_QUOTE.
 *
 * Known limitation (a good discussion topic for your write-up): the result is
 * just strings, so a QUOTED operator such as ">" cannot be told apart from a
 * real one later. The parser treats it as an operator. */
int tokenize(const char *line, struct strvec *out);

#endif /* LEXER_H */
