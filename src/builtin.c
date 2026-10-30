/*
 * builtin.c - Milestone 1: recognising the built-in command names.
 * Contract: include/builtin.h
 */
#include "builtin.h"

#include <stddef.h>
#include <string.h>

/* TODO: define a struct { const char *name; enum builtin_id id; } and a static
 * table with one entry per built-in. Both functions below can then be a simple
 * loop over the table. */

struct LookupStruct{
    const char *name;
    enum builtin_id id;
};

static const struct LookupStruct lookup_table[] = {
    {"exit", BUILTIN_EXIT},
    {"cd", BUILTIN_CD},
    {"pwd", BUILTIN_PWD},
    {"help", BUILTIN_HELP},
    {"jobs", BUILTIN_JOBS},
    {"history", BUILTIN_HISTORY}
};

enum builtin_id builtin_lookup(const char *name)
{
    /* TODO */
    if(name != NULL){
        int n = sizeof(lookup_table) / sizeof(lookup_table[0]);
        for(int i = 0; i < n; i++){
            if(strcmp(name, lookup_table[i].name) == 0){
                return lookup_table[i].id;
            }
        }
    }
    return BUILTIN_NONE;
    // if(name != NULL){
    //     if(strcmp(name, "exit") == 0){
    //         return BUILTIN_EXIT;
    //     } else if(strcmp(name, "cd") == 0){
    //         return BUILTIN_CD;
    //     } else if(strcmp(name, "pwd") == 0){
    //         return BUILTIN_PWD;
    //     } else if(strcmp(name, "help") == 0){
    //         return BUILTIN_HELP;
    //     } else if(strcmp(name, "jobs") == 0){
    //         return BUILTIN_JOBS;
    //     } else if(strcmp(name, "history") == 0){
    //         return BUILTIN_HISTORY;
    //     }
    // }
    // return BUILTIN_NONE;
}

const char *builtin_name(enum builtin_id id)
{
    /* TODO */
    int n = sizeof(lookup_table) / sizeof(lookup_table[0]);
    for(int i = 0; i < n; i++){
        if(id == lookup_table[i].id){
            return lookup_table[i].name;
        }
    }
    return NULL;
}
