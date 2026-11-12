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
    size_t string_len = my_strlen(line);
    int count = 0;
    char *buffer = malloc(string_len+1);
    if(buffer == NULL){
        printf("Failed malloc operation:\n");
        return LEX_ERR_NOMEM;
    }
    char *original_buffer = buffer;
    char *token_start = NULL;
    while(*line){
        if(isspace(*line)){
            if(token_start != NULL){
                // printf("Sentence spacing\n");
                count++;
                *buffer = '\0';
                int result = strvec_push(out, original_buffer);
                if(result != 0){
                    (*out).len = 0;
                }
                buffer = original_buffer;
                token_start = NULL;
            }
        } else {
            if(token_start == NULL){
                token_start = (char *)line;
                // printf("Token start: %p\n", token_start);
            }
            // printf("Char %c\n", *line);
            *buffer = *line;
            buffer++;
        }
        line++;
    }
    if(token_start != NULL){
        count++;
        *buffer = '\0';
        int result = strvec_push(out, original_buffer);
        if(result != 0){
            (*out).len = 0;
        }
    }
    buffer = original_buffer;
    memset(buffer, '\0', string_len+1);
    free(buffer);
    return count;
}
