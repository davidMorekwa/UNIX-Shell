/*
 * strvec.c - Milestone 2: a growable array of strings.
 * Contract: include/strvec.h  (read the growth rule and the failure rules!)
 *
 * Use my_strdup from mystring.h to copy strings.
 */
#include "strvec.h"

#include <stdlib.h>
#include <stdio.h>

#include "mystring.h"

void strvec_init(struct strvec *v)
{
    /* TODO */
    (*v).len = 0;
    (*v).cap = 0;
    (*v).items = NULL;
    // (void)v;
}

int strvec_push(struct strvec *v, const char *s)
{
    /* TODO:
     *   1. reject NULL arguments
     *   2. make the copy of s
     *   3. if len == cap, grow: first allocation holds 4 slots, afterwards
     *      double. Assign realloc's result to a TEMPORARY pointer; only if it
     *      is not NULL do you store it in v->items.
     *   4. store the copy and bump len
     * If anything fails, free what you allocated in this call and return -1. */
    if(v == NULL || s == NULL){
        return -1;
    }
    // printf("Old s: %p \n", s);
    char *new_s = my_strdup(s);
    if(new_s == NULL){
        return -1;
    }
    // printf("New s: %p \n", new_s);
    if((*v).len == (*v).cap){
        // TODO: Allocate memory for 4 string
        // printf("Allocating new memory\n");
        int multiple = (*v).cap == 0 ? 4 : (2*(*v).cap);
        char **new_memory = realloc((*v).items, (multiple * sizeof(char *)));
        if(new_memory == NULL){
            free(new_s);
            return -1;
        }
        (*v).items = new_memory;
        (*v).cap = multiple;
    }
    // printf("Items pointer: %p\n", (*v).items);
    (*v).items[(*v).len] = new_s;
    (*v).len++;
    return 0;
}

const char *strvec_get(const struct strvec *v, size_t i)
{
    /* TODO */
    if(i >= (*v).len){
        return NULL;
    }
    return (*v).items[i];
}

char *strvec_pop(struct strvec *v)
{
    /* TODO: the caller becomes the owner of the returned string. */
    if((*v).items == NULL || (*v).len == 0){
        return NULL;
    }
    // printf("Len: %zu\n", (*v).len);
    char *pop_item = (*v).items[((*v).len-1)];
    (*v).len--;
    // printf("Len: %zu\n", (*v).len);
    return pop_item;
}

void strvec_clear(struct strvec *v)
{
    /* TODO: free the strings, set len to 0, KEEP items and cap. */
    for(int i = 0; i < (int)(*v).len; i++){
        free((*v).items[i]);
    }
    (*v).len = 0;
}

void strvec_free(struct strvec *v)
{
    /* TODO: free the strings and the array, then reset to the init state so a
     * second call (or a later push) is safe. */
    for(int i = 0; i < (int)(*v).len; i++){
        free((*v).items[i]);
    }
    (*v).items = NULL;
    (*v).len = 0;
    (*v).cap = 0;
}
