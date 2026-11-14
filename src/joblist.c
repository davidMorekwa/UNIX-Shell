/*
 * joblist.c - Milestone 2: a linked list of background jobs.
 * Contract: include/joblist.h
 *
 * Hint for add and remove: a "pointer to pointer" (struct job **link) that
 * points at either jl->head or some node's `next` field lets you handle the
 * first node, a middle node and the last node with the SAME code, with no
 * special case for the head.
 */
#include "joblist.h"

#include <stdio.h>
#include <stdlib.h>

#include "mystring.h"

void joblist_init(struct joblist *jl)
{
    /* TODO */
    (*jl).head = NULL;
    (*jl).next_id = 1;
}

int joblist_add(struct joblist *jl, pid_t pid, const char *cmdline)
{
    /* TODO: malloc the node, my_strdup the text; if the second step fails, free
     * the node. Append at the END of the list. Only bump next_id on success. */
    if(jl == NULL){
        return -1;
    }
    struct job *new_job = malloc(sizeof(struct job));
    if(new_job == NULL){
        return -1;
    }
    (*new_job).id = (*jl).next_id;
    (*new_job).pid = pid;
    char *cmd_dst = my_strdup(cmdline);
    if(cmd_dst == NULL){
        free(new_job);
        return -1;
    }
    (*new_job).cmdline = cmd_dst;
    (*new_job).next = NULL;
    if((*jl).head == NULL){
        (*jl).head = new_job;
    } else {
        struct job *curr_job = (*jl).head;
        while((*curr_job).next != NULL){
            curr_job = (*curr_job).next;
        }
        (*curr_job).next = new_job;
    }
    (*jl).next_id++;
    return (*new_job).id;
}

struct job *joblist_find_pid(const struct joblist *jl, pid_t pid)
{
    /* TODO */
    if(jl == NULL || (*jl).head == NULL){
        return NULL;
    }
    struct job *curr_job = (*jl).head;
    while(curr_job != NULL){
        if((*curr_job).pid == pid){
            return curr_job;
        }
        curr_job = (*curr_job).next;
    }
    return NULL;
}

int joblist_remove_pid(struct joblist *jl, pid_t pid)
{
    /* TODO: unlink the node FIRST, then free its cmdline and the node itself. */
    if(jl == NULL || (*jl).head == NULL){
        return -1;
    }
    struct job *curr_job = (*jl).head;
    struct job *prev_job = NULL;
    while(curr_job != NULL){
        if((*curr_job).pid == pid){
            // if its head node
            if(prev_job == NULL && (*curr_job).next == NULL){
                (*jl).head = NULL;
            } else if(prev_job == NULL && (*curr_job).next != NULL){
                (*jl).head = (*curr_job).next;
            } else{
                (*prev_job).next = (*curr_job).next;
            }
            free((*curr_job).cmdline);
            free(curr_job);
            return 0;
        }
        prev_job = curr_job;
        curr_job = (*curr_job).next;
    }
    return -1;
}

int joblist_count(const struct joblist *jl)
{
    /* TODO */
    if(jl == NULL || (*jl).head == NULL){
        return 0;
    }
    int count = 0;
    struct job *curr_job = (*jl).head;
    while(curr_job != NULL){
        count++;
        curr_job = (*curr_job).next;
    }
    return count;
}

int joblist_format(const struct joblist *jl, char *buf, size_t size)
{
    /* TODO: one snprintf per job, appended after the previous output, counting
     * the FULL length of every line even once the buffer is full. Take care
     * that size may be 0 and that an empty list must still give "". */
    if(jl == NULL || (*jl).head == NULL){
        if(buf != NULL && size > 0){
            *buf = '\0';
        }
        return 0;
    }
    struct job *curr_job = (*jl).head;
    int total_len = 0;
    while(curr_job != NULL){
        char *dest = NULL;
        size_t dest_size = 0;
        if (buf != NULL && (size_t)total_len < size) {
            dest = buf + total_len;
            dest_size = size - total_len;
        }
        int n = snprintf(dest, dest_size, "[%d] %d %s\n", (*curr_job).id, (*curr_job).pid, (*curr_job).cmdline);
        total_len += n;
        curr_job = (*curr_job).next;
    }
    return total_len;
}

void joblist_free(struct joblist *jl)
{
    /* TODO: read `next` BEFORE freeing a node. Finish with joblist_init. */
    struct job *curr_job = (*jl).head;
    struct job *del_job = NULL;
    while(curr_job != NULL){
        del_job = curr_job;
        curr_job = (*curr_job).next;
        free((*del_job).cmdline);
        free(del_job);
    }
    (*jl).head = NULL;
    (*jl).next_id = 1;

}
