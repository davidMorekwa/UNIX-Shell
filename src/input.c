/*
 * input.c - Milestone 1: reading one line of input, and trimming it.
 * Contract: include/input.h
 */
#include "input.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

int read_line(FILE *in, char *buf, size_t size)
{
    /* TODO: fgets() into buf. If it returns NULL -> READ_EOF.
     * If the line ended with '\n', remove it and return the length.
     * If not, work out WHY there is no '\n': the line exactly filled the buffer,
     * it was the last line of the file, or it was too long (then discard the
     * rest of it so the next call starts on the next line). */
    // (void)in;
    // (void)buf;
    // (void)size;

    if(in != NULL && buf != NULL){
        // printf("Null checks passed \n");
        if(fgets(buf, size, in) != NULL){
            // printf("Input read %s \n", buf);

            // TODO: Handle buffer limit reached
            if(strlen(buf) >= size-1){
                // TODO: 1. input fits
                int next_char;
                next_char = fgetc(in);
                // printf("Next char: %c \n", next_char);
                if(next_char != '\n'){
                    // TODO: 2. Input doesn't fit
                    // printf("Buffer overflow \n");
                    while((next_char = fgetc(in)) != EOF && next_char !='\n');
                    return READ_TOO_LONG;
                }
            }
            // if(strlen(buf) == size-1 && strchr(buf, '\n') == NULL){
            //     int char_next = fgetc(in);
            //     printf("NExt character %c \n", char_next);
            //     if(char_next != '\n' && char_next != EOF){
            //         printf("Buffer overflow detected \n");
            //         int overflow_chars;
            //         while((overflow_chars = fgetc(in)) != '\n' && overflow_chars != EOF);
            //         return READ_TOO_LONG;
            //     } else if(strchr(buf, '\n') == NULL){
            //         printf("Buffer overflow 2");
            //         return READ_TOO_LONG;
            //     }
            // }
            buf[strcspn(buf, "\n")] = '\0';
            // printf("Buf: %s \n", buf);
            return strlen(buf);
        }
    }
    return READ_EOF;
}

char *str_trim(char *s)
{
    /* TODO: skip leading whitespace (isspace needs an unsigned char!), then
     * walk back from the end to cut trailing whitespace. */

    if (s == NULL) {
        return NULL;
    }
    // if(s[0] == '\0'){
    //     printf("Empty string detected\n");
    // }

    // TODO: 1. Find first non_whitespace character
    char *first_non_whitespace = NULL;
    for(int i = 0; i < (int)strlen(s); i++){
        if(!isspace(s[i])){
            // printf("Char:%02X\n", s[i]);
            first_non_whitespace = &s[i];
            break;
        }
    }
    if(first_non_whitespace == NULL){
        // printf("Empty string detected");
        return "";
    }
    // printf("First non whitespace character:%p -> %c\n", first_non_whitespace, *first_non_whitespace);
    // TODO: 2. Find last non_whitespace
    char *last_non_whitespace = NULL;
    for(int i = (int)strlen(s)-1; i >= 0; i--){
        if(!isspace(s[i])){
            last_non_whitespace = &s[i];
            break;
        }
    }
    // // TODO: Add null termination
    *(last_non_whitespace+1) = '\0';
    // printf("Last non whitespace char:%p -> %c\nNull termination:%p -> %02X\n", last_non_whitespace, *last_non_whitespace, (last_non_whitespace+1), *(last_non_whitespace+1));
    // printf("size: %d\n", (int)(last_non_whitespace - first_non_whitespace + 2));
    // if(first_non_whitespace != last_non_whitespace){
    //     memmove(s, first_non_whitespace, ((last_non_whitespace - first_non_whitespace) + 2));
    // }
    // for(int i = 0; i < (int)strlen(s); i++){
    //     printf("%c -> %02X -> %p\n",s[i], s[i], &s[i]);
    // }
    // printf("Final string:%s\n",first_non_whitespace);
    return first_non_whitespace;
    // return "";
    // return "hi";
}

