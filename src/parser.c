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
#include <stdio.h>
#include <command.h>

#include "mystring.h"

void add_to_pipeline(struct command curr_command, struct pipeline *out);
struct command reset_command_struct(struct command *cmd);
int check_syntax_error(struct command *cmd);


int parse_pipeline(const struct strvec *tokens, struct pipeline *out)
{
    pipeline_init(out);
    if(tokens == NULL || (*tokens).len == 0){
        return PARSE_EMPTY;
    }
    struct command current_command;
    command_init(&current_command);

    int has_infile = 0;
    int has_outfile = 0;
    int append = 0;
    int expecting_filename = 0; // 1 for infile, 2 for outfile

    for(int i = 0; i < (int)(*tokens).len; i++){
        char *item = (char *)(*tokens).items[i];
        // printf("Item: %s\n", item);

        if(expecting_filename != 0){
            if(my_strcmp(item, "<") == 0 || my_strcmp(item, ">") == 0 ||
               my_strcmp(item, ">>") == 0 || my_strcmp(item, "|") == 0 ||
               my_strcmp(item, "&") == 0){
                return PARSE_ERR_SYNTAX;
            }

            if(my_strlen(item) > MAX_PATH_LEN){
                return PARSE_ERR_TOO_LONG;
            }

            int res = (expecting_filename == 1)
                ? command_set_infile(&current_command, item)
                : command_set_outfile(&current_command, item, append);

            if(res != 0){
                // printf("Command error\n");
                return PARSE_ERR_SYNTAX;
            }
            expecting_filename = 0;
            continue;
        }

        if(my_strcmp(item, "<") == 0){
            // Input redirection is ONLY allowed on the first command
            if((*out).ncmds > 0 || i == (int)(*tokens).len - 1 || has_infile){
                // printf("[-] Duplicate or misplaced '<'\n");
                return PARSE_ERR_SYNTAX;
            }
            has_infile = 1;
            expecting_filename = 1;
            continue;
        } else if(my_strcmp(item, ">") == 0){
            if(i == (int)(*tokens).len - 1 || has_outfile){
                // printf("[-] Duplicate or misplaced '>'\n");
                return PARSE_ERR_SYNTAX;
            }
            has_outfile = 1;
            append = 0;
            expecting_filename = 2;
            continue;
        } else if(my_strcmp(item, ">>") == 0){
            if(current_command.argc == 0 || i == (int)(*tokens).len - 1 || has_outfile){
                // printf("[-] Duplicate or misplaced '>>'\n");
                return PARSE_ERR_SYNTAX;
            }
            has_outfile = 1;
            append = 1;
            expecting_filename = 2;
            continue;
        } else if(my_strcmp(item, "|") == 0){
            if(current_command.argc == 0 || i == (int)(*tokens).len - 1){
                // printf("[-] Misplaced '|'\n");
                return PARSE_ERR_SYNTAX;
            }
            // Output redirection is NOT allowed before a pipe (must be on the last command)
            if(has_outfile){
                return PARSE_ERR_SYNTAX;
            }
            if((*out).ncmds >= MAX_CMDS){
                return PARSE_ERR_TOO_MANY_CMDS;
            }
            add_to_pipeline(current_command, out);
            command_init(&current_command);
            has_infile = 0;
            has_outfile = 0;
            append = 0;
            continue;
        } else if(my_strcmp(item, "&") == 0){
            if(current_command.argc == 0 || i < (int)(*tokens).len - 1){
                // printf("[-] Misplaced '&'\n");
                return PARSE_ERR_SYNTAX;
            }
            if(!has_infile || !has_outfile) {
                (*out).background = 1;
                continue;
            }
        }

        if(current_command.argc >= MAX_ARGS){
            return PARSE_ERR_TOO_MANY_ARGS;
        }
        if(my_strlen(item) > MAX_ARG_LEN){
            return PARSE_ERR_TOO_LONG;
        }
        if(command_add_arg(&current_command, item) != 0){
            printf("[-]Command add args failed\n");
        }
        // printf("Command struct:\n\targc -> %d\n\targv -> %s\n",
        //        current_command.argc,
        //        current_command.argv[current_command.argc - 1]);
    }

    if(expecting_filename != 0 || current_command.argc == 0){
        return PARSE_ERR_SYNTAX;
    }
    if((*out).ncmds >= MAX_CMDS){
        return PARSE_ERR_TOO_MANY_CMDS;
    }
    add_to_pipeline(current_command, out);
    return PARSE_OK;
}


void add_to_pipeline(struct command curr_command, struct pipeline *out){
    // printf("[+] Adding to pipeline \n");
    if(out != NULL){
        (*out).cmds[(*out).ncmds] = curr_command;
        (*out).ncmds++;
    }
    // printf("[+][+]Pipeline:\n\tncmds: %d\n", (*out).ncmds);
}
struct command reset_command_struct(struct command *cmd){
    if(cmd != NULL){
        printf("[+]command struct reset:\n\targc -> %d\n", (*cmd).argc);
    }
    command_init(cmd);
    return *cmd;
}

