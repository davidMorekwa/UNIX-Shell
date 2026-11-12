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
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "mystring.h"

int tokenize(const char *line, struct strvec *out)
{
    if (!line || !out) {
        return -1;
    }

    strvec_init(out);

    size_t len = my_strlen(line);
    char *buf = malloc(len + 1);
    if (!buf) {
        return LEX_ERR_NOMEM;
    }
    size_t buf_len = 0;

    bool in_single = false;
    bool in_double = false;
    bool token_started = false;

    const char *p = line;
    while (*p) {
        if (*p == '\'' && !in_double) {
            in_single = !in_single;
            token_started = true;
            p++;
        } else if (*p == '"' && !in_single) {
            in_double = !in_double;
            token_started = true;
            p++;
        } else if (!in_single && !in_double && isspace((unsigned char)*p)) {
            if (token_started) {
                buf[buf_len] = '\0';
                if (strvec_push(out, buf) != 0) {
                    free(buf);
                    strvec_free(out);
                    out->len = 0;
                    return LEX_ERR_NOMEM;
                }
                buf_len = 0;
                token_started = false;
            }
            p++;
        } else if (!in_single && !in_double && (*p == '|' || *p == '<' || *p == '>' || *p == '&')) {
            if (token_started) {
                buf[buf_len] = '\0';
                if (strvec_push(out, buf) != 0) {
                    free(buf);
                    strvec_free(out);
                    out->len = 0;
                    return LEX_ERR_NOMEM;
                }
                buf_len = 0;
                token_started = false;
            }

            char op_str[3];
            if (*p == '>' && *(p + 1) == '>') {
                op_str[0] = '>';
                op_str[1] = '>';
                op_str[2] = '\0';
                p += 2;
            } else {
                op_str[0] = *p;
                op_str[1] = '\0';
                p += 1;
            }

            if (strvec_push(out, op_str) != 0) {
                free(buf);
                strvec_free(out);
                out->len = 0;
                return LEX_ERR_NOMEM;
            }
        } else {
            buf[buf_len++] = *p;
            token_started = true;
            p++;
        }
    }

    if (in_single || in_double) {
        free(buf);
        strvec_free(out);
        out->len = 0;
        return LEX_ERR_QUOTE;
    }

    if (token_started) {
        buf[buf_len] = '\0';
        if (strvec_push(out, buf) != 0) {
            free(buf);
            strvec_free(out);
            out->len = 0;
            return LEX_ERR_NOMEM;
        }
    }

    free(buf);
    return (int)out->len;
}
