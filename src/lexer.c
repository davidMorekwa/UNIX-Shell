/*
 * lexer.c - Milestone 2: splitting a line into tokens.
 * Contract: include/lexer.h  (all the rules and examples are there)
 *
 * RULES (checked by "make lint"): no strtok, strsep or sscanf. Do the
 * splitting yourself, one character at a time.
 *
 * Suggested design: walk the line with an index i. Keep ONE scratch buffer
 * (malloc'ed once, as long as the whole line + 1: no token can be longer) and
 * build the current token in it, then strvec_push the finished token. Three
 * cases at each step:
 *     whitespace        -> skip it
 *     operator char     -> the token is that operator (or ">>")
 *     anything else     -> a word: keep reading until unquoted whitespace or an
 *                          operator; when you see a quote, copy everything up
 *                          to the matching quote (without the quote characters)
 * Whatever goes wrong, free the scratch buffer, empty `out`, and return the
 * right error code.
 */
#include "lexer.h"

#include <stdlib.h>

#include "mystring.h"

int tokenize(const char *line, struct strvec *out)
{
    /* TODO */
    (void)line;
    (void)out;
    return LEX_ERR_NOMEM;
}
