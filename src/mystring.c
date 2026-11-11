/*
 * mystring.c - Milestone 2: your own <string.h> functions.
 * Contract: include/mystring.h  (behave EXACTLY like the libc versions)
 *
 * RULES (checked by "make lint"):
 *   - do not include <string.h>
 *   - do not call any libc string or memory function from this file
 *   - <stdlib.h> is fine, for malloc
 * Also avoid writing the libc function names followed by an opening
 * parenthesis, even inside comments: the lint check is a plain text search.
 *
 * Hint: work with pointers, not indexes, in at least a couple of these. Try
 * writing my_strlen with a pointer that walks to the terminating '\0'.
 */
#include "mystring.h"
#include <stdio.h>
#include <stdlib.h>

size_t my_strlen(const char *s)
{
    /* TODO */
    int count = 0;
    while(s[count] != '\0'){
        count ++;
    }
    return count;
}

int my_strcmp(const char *a, const char *b)
{
    /* TODO: compare as UNSIGNED char (cast before subtracting). */
    const unsigned char *unsigned_a = (const unsigned char *)a;
    const unsigned char *unsigned_b = (const unsigned char *)b;
    while(*unsigned_a && (*unsigned_a == *unsigned_b)){
        unsigned_a++;
        unsigned_b++;
    }
    return (int)*unsigned_a - (int)*unsigned_b;
}

int my_strncmp(const char *a, const char *b, size_t n)
{
    /* TODO: like my_strcmp, but give up after n characters. */
    const unsigned char *unsigned_a = (const unsigned char *)a;
    const unsigned char *unsigned_b = (const unsigned char *)b;
    int count = 0;
    while(count < (int)n){
        // printf("A: %p -> %c \n", unsigned_a, *unsigned_a);
        // printf("B: %p -> %c \n", unsigned_b, *unsigned_b);
        if(!(*unsigned_a == *unsigned_b) || *unsigned_a == '\0'){
            // printf("same chars\n");
            // printf("Different chars \n");
            // printf("%d\n", (int)*unsigned_a-(int)*unsigned_b);
            return (int)*unsigned_a - (int)*unsigned_b;
        }
        count++;
        unsigned_a++;
        unsigned_b++;
    }
    return 0;
}

char *my_strcpy(char *dst, const char *src)
{
    /* TODO */
    char *original_dst = dst;
    while(*src != '\0'){
        // printf("src: %p -> %c\n", src, *src);
        *dst = *src;
        // printf("dst: %p -> %c\n", dst, *dst);
        dst++;
        src++;
    }
    *dst = '\0';
    return original_dst;
}

char *my_strncpy(char *dst, const char *src, size_t n)
{
    /* TODO: copy up to n chars; if src ends first, pad dst with '\0' up to n
     * bytes. Do not write more than n bytes. */
    char *original_dst = dst;
    int count = 0;
    while(count < (int)n){
        if(*src != '\0'){
            *dst = *src;
            src++;
            dst++;
        } else{
            *dst = '\0';
            dst++;
        }
        count++;
    }
    return original_dst;
}

char *my_strchr(const char *s, int c)
{
    /* TODO: remember that c is converted to char, and that the terminating
     * '\0' counts as part of the string. */
    char *location = NULL;
    while(*s != '\0'){
        if(*s == c || *s == (char)c){
            location = (char *)s;
            break;
        }
        s++;
    }
    if(c == '\0'){
        location = (char *)s;
    }
    // printf("Found: %p\n", location);
    return location;
}

char *my_strdup(const char *s)
{
    /* TODO: NULL in -> NULL out; malloc(length + 1); check the malloc result
     * BEFORE copying; use your own my_strlen and my_strcpy. */
    if(s == NULL){
        return NULL;
    }
    size_t string_length = my_strlen(s);
    char *dst = malloc((string_length+1));
    if(dst != NULL){
        my_strcpy(dst, s);
        return dst;
    }
    return NULL;
}
