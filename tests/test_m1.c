/* test_m1.c - unit tests for Milestone 1 (Week 1): command, shell_state,
 * builtin, input. Run with:  make test1   */
#include "testlib.h"

#include "builtin.h"
#include "command.h"
#include "input.h"
#include "shell_state.h"

/* Build a string of n copies of ch. */
static void fill(char *buf, size_t n, char ch)
{
    memset(buf, ch, n);
    buf[n] = '\0';
}

/* ======================= command.c ======================= */

static void test_command_init_clears_everything(void)
{
    struct command c;
    memset(&c, 0xFF, sizeof c); /* garbage, so init must really set every field */
    command_init(&c);
    CHECK_INT(c.argc, 0);
    CHECK_INT(c.argv[0][0], 0);
    CHECK_INT(c.argv[MAX_ARGS - 1][0], 0);
    CHECK_INT(c.infile[0], 0);
    CHECK_INT(c.outfile[0], 0);
    CHECK_INT(c.append, 0);
}

static void test_add_arg_basic(void)
{
    struct command c;
    command_init(&c);
    CHECK_INT(command_add_arg(&c, "ls"), 0);
    CHECK_INT(command_add_arg(&c, "-l"), 0);
    CHECK_INT(c.argc, 2);
    CHECK_STR(c.argv[0], "ls");
    CHECK_STR(c.argv[1], "-l");
}

static void test_add_arg_copies_the_string(void)
{
    struct command c;
    char word[16] = "hello";
    command_init(&c);
    CHECK_INT(command_add_arg(&c, word), 0);
    strcpy(word, "CHANGED");
    CHECK_STR(c.argv[0], "hello");
}

static void test_add_arg_empty_string_is_a_word(void)
{
    struct command c;
    command_init(&c);
    CHECK_INT(command_add_arg(&c, ""), 0);
    CHECK_INT(c.argc, 1);
    CHECK_STR(c.argv[0], "");
}

static void test_add_arg_null_is_rejected(void)
{
    struct command c;
    command_init(&c);
    CHECK_INT(command_add_arg(&c, NULL), -1);
    CHECK_INT(c.argc, 0);
}

static void test_add_arg_full_command(void)
{
    struct command c;
    char w[8];
    int i;
    command_init(&c);
    for (i = 0; i < MAX_ARGS; i++) {
        snprintf(w, sizeof w, "w%d", i);
        CHECK_INT(command_add_arg(&c, w), 0);
    }
    CHECK_INT(c.argc, MAX_ARGS);
    CHECK_INT(command_add_arg(&c, "one-too-many"), -1);
    CHECK_INT(c.argc, MAX_ARGS);
    CHECK_STR(c.argv[MAX_ARGS - 1], "w15");
}

static void test_add_arg_length_limit(void)
{
    struct command c;
    char longest[MAX_ARG_LEN + 1];
    char toolong[MAX_ARG_LEN + 2];
    command_init(&c);

    fill(longest, MAX_ARG_LEN, 'x');
    fill(toolong, MAX_ARG_LEN + 1, 'y');

    CHECK_INT(command_add_arg(&c, longest), 0);         /* exactly the maximum */
    CHECK_INT(command_add_arg(&c, toolong), -1);        /* one more is too long */
    CHECK_INT(c.argc, 1);                               /* ...and not stored    */
    CHECK_INT(command_add_arg(&c, "next"), 0);
    CHECK_STR(c.argv[0], longest);                      /* no overflow/clobbering */
    CHECK_STR(c.argv[1], "next");
}

static void test_set_infile(void)
{
    struct command c;
    char longest[MAX_PATH_LEN + 1];
    char toolong[MAX_PATH_LEN + 2];
    command_init(&c);
    fill(longest, MAX_PATH_LEN, 'p');
    fill(toolong, MAX_PATH_LEN + 1, 'q');

    CHECK_INT(command_set_infile(&c, "in.txt"), 0);
    CHECK_STR(c.infile, "in.txt");

    CHECK_INT(command_set_infile(&c, toolong), -1);
    CHECK_STR(c.infile, "in.txt");                      /* unchanged on failure */
    CHECK_INT(command_set_infile(&c, ""), -1);          /* "" would mean "none" */
    CHECK_STR(c.infile, "in.txt");
    CHECK_INT(command_set_infile(&c, NULL), -1);
    CHECK_STR(c.infile, "in.txt");

    CHECK_INT(command_set_infile(&c, longest), 0);      /* exact maximum is fine */
    CHECK_STR(c.infile, longest);
}

static void test_set_outfile_modes(void)
{
    struct command c;
    command_init(&c);

    CHECK_INT(command_set_outfile(&c, "out.txt", 0), 0);
    CHECK_STR(c.outfile, "out.txt");
    CHECK_INT(c.append, 0);

    CHECK_INT(command_set_outfile(&c, "log.txt", 1), 0);
    CHECK_STR(c.outfile, "log.txt");
    CHECK_INT(c.append, 1);

    CHECK_INT(command_set_outfile(&c, "other.txt", 7), 0);  /* any nonzero -> 1 */
    CHECK_INT(c.append, 1);
}

static void test_set_outfile_failure_changes_nothing(void)
{
    struct command c;
    char toolong[MAX_PATH_LEN + 2];
    command_init(&c);
    fill(toolong, MAX_PATH_LEN + 1, 'q');

    CHECK_INT(command_set_outfile(&c, "keep.txt", 1), 0);
    CHECK_INT(command_set_outfile(&c, toolong, 0), -1);
    CHECK_STR(c.outfile, "keep.txt");
    CHECK_INT(c.append, 1);                             /* append untouched too */
    CHECK_INT(command_set_outfile(&c, "", 0), -1);
    CHECK_INT(c.append, 1);
    CHECK_INT(command_set_outfile(&c, NULL, 0), -1);
    CHECK_INT(c.append, 1);
}

static void test_argv_ptrs(void)
{
    struct command c;
    char *out[MAX_ARGS + 1];
    command_init(&c);
    command_add_arg(&c, "a");
    command_add_arg(&c, "b");
    command_add_arg(&c, "c");
    memset(out, 0xFF, sizeof out);
    command_argv_ptrs(&c, out);
    CHECK(out[0] == c.argv[0]);   /* pointers INTO the struct, not copies */
    CHECK(out[1] == c.argv[1]);
    CHECK(out[2] == c.argv[2]);
    CHECK(out[3] == NULL);
}

static void test_argv_ptrs_empty_command(void)
{
    struct command c;
    char *out[MAX_ARGS + 1];
    command_init(&c);
    memset(out, 0xFF, sizeof out);
    command_argv_ptrs(&c, out);
    CHECK(out[0] == NULL);
}

static void test_argv_ptrs_full_command(void)
{
    struct command c;
    char *out[MAX_ARGS + 1];
    int i;
    command_init(&c);
    for (i = 0; i < MAX_ARGS; i++) command_add_arg(&c, "x");
    memset(out, 0xFF, sizeof out);
    command_argv_ptrs(&c, out);
    CHECK(out[MAX_ARGS - 1] == c.argv[MAX_ARGS - 1]);
    CHECK(out[MAX_ARGS] == NULL);  /* the extra slot is what makes room for NULL */
}

static void test_format_words_and_redirects(void)
{
    struct command c;
    char buf[200];

    command_init(&c);
    command_add_arg(&c, "ls");
    CHECK_INT(command_format(&c, buf, sizeof buf), 2);
    CHECK_STR(buf, "ls");

    command_add_arg(&c, "-l");
    command_add_arg(&c, "/tmp");
    CHECK_INT(command_format(&c, buf, sizeof buf), 10);
    CHECK_STR(buf, "ls -l /tmp");

    command_init(&c);
    command_add_arg(&c, "sort");
    command_set_infile(&c, "in.txt");
    CHECK_STR((command_format(&c, buf, sizeof buf), buf), "sort < in.txt");

    command_init(&c);
    command_add_arg(&c, "echo");
    command_add_arg(&c, "hi");
    command_set_outfile(&c, "out", 0);
    CHECK_STR((command_format(&c, buf, sizeof buf), buf), "echo hi > out");

    command_set_outfile(&c, "log", 1);
    CHECK_STR((command_format(&c, buf, sizeof buf), buf), "echo hi >> log");

    command_init(&c);
    command_add_arg(&c, "cat");
    command_set_outfile(&c, "b", 0); /* set out first: order in text is fixed */
    command_set_infile(&c, "a");
    CHECK_INT(command_format(&c, buf, sizeof buf), 11);
    CHECK_STR(buf, "cat < a > b");
}

static void test_format_empty_command(void)
{
    struct command c;
    char buf[16] = "garbage";
    command_init(&c);
    CHECK_INT(command_format(&c, buf, sizeof buf), 0);
    CHECK_STR(buf, "");
}

static void test_format_truncates_like_snprintf(void)
{
    struct command c;
    char buf[32];
    command_init(&c);
    command_add_arg(&c, "hello");
    command_add_arg(&c, "world"); /* "hello world" is 11 characters */

    memset(buf, 'Z', sizeof buf);
    CHECK_INT(command_format(&c, buf, 5), 11);   /* returns the FULL length */
    CHECK_STR(buf, "hell");                      /* but only 4 chars + '\0' */
    CHECK_INT(buf[5], 'Z');                      /* nothing written past size */

    memset(buf, 'Z', sizeof buf);
    CHECK_INT(command_format(&c, buf, 1), 11);
    CHECK_STR(buf, "");

    CHECK_INT(command_format(&c, NULL, 0), 11);  /* size 0: no write, no crash */
}

static void test_format_exact_fit_boundary(void)
{
    struct command c;
    char buf[32];
    command_init(&c);
    command_add_arg(&c, "hello");
    command_add_arg(&c, "world");

    memset(buf, 'Z', sizeof buf);
    CHECK_INT(command_format(&c, buf, 12), 11);  /* size = len+1: fits */
    CHECK_STR(buf, "hello world");

    memset(buf, 'Z', sizeof buf);
    CHECK_INT(command_format(&c, buf, 11), 11);  /* size = len: loses last char */
    CHECK_STR(buf, "hello worl");
}

static void test_format_truncation_inside_redirect(void)
{
    struct command c;
    char buf[32];
    command_init(&c);
    command_add_arg(&c, "cat");
    command_set_infile(&c, "input.txt");       /* "cat < input.txt" = 15 chars */
    memset(buf, 'Z', sizeof buf);
    CHECK_INT(command_format(&c, buf, 8), 15);
    CHECK_STR(buf, "cat < i");
}

static void test_pipeline_init(void)
{
    struct pipeline pl;
    int i;
    memset(&pl, 0xFF, sizeof pl);
    pipeline_init(&pl);
    CHECK_INT(pl.ncmds, 0);
    CHECK_INT(pl.background, 0);
    for (i = 0; i < MAX_CMDS; i++) {
        CHECK_INT(pl.cmds[i].argc, 0);
        CHECK_INT(pl.cmds[i].infile[0], 0);
        CHECK_INT(pl.cmds[i].outfile[0], 0);
        CHECK_INT(pl.cmds[i].append, 0);
    }
}

/* ======================= shell_state.c ======================= */

static void test_state_init(void)
{
    struct shell_state st;
    memset(&st, 0xFF, sizeof st);
    shell_state_init(&st);
    CHECK_INT(st.last_status, 0);
    CHECK_INT(st.running, 1);
}

static void test_state_set_status_keeps_low_8_bits(void)
{
    struct shell_state st;
    shell_state_init(&st);
    shell_state_set_status(&st, 0);    CHECK_INT(st.last_status, 0);
    shell_state_set_status(&st, 7);    CHECK_INT(st.last_status, 7);
    shell_state_set_status(&st, 255);  CHECK_INT(st.last_status, 255);
    shell_state_set_status(&st, 256);  CHECK_INT(st.last_status, 0);
    shell_state_set_status(&st, 257);  CHECK_INT(st.last_status, 1);
    shell_state_set_status(&st, -1);   CHECK_INT(st.last_status, 255);
    shell_state_set_status(&st, 1000); CHECK_INT(st.last_status, 232);
    CHECK_INT(st.running, 1);          /* must not disturb other fields */
}

static void test_prompt_success(void)
{
    struct shell_state st;
    char buf[64];
    shell_state_init(&st);
    CHECK_INT(shell_state_format_prompt(&st, buf, sizeof buf), 6);
    CHECK_STR(buf, "mysh> ");
}

static void test_prompt_shows_nonzero_status(void)
{
    struct shell_state st;
    char buf[64];
    shell_state_init(&st);
    shell_state_set_status(&st, 127);
    CHECK_INT(shell_state_format_prompt(&st, buf, sizeof buf), 12);
    CHECK_STR(buf, "[127] mysh> ");
    shell_state_set_status(&st, 1);
    CHECK_INT(shell_state_format_prompt(&st, buf, sizeof buf), 10);
    CHECK_STR(buf, "[1] mysh> ");
}

static void test_prompt_truncates_like_snprintf(void)
{
    struct shell_state st;
    char buf[16];
    shell_state_init(&st);
    memset(buf, 'Z', sizeof buf);
    CHECK_INT(shell_state_format_prompt(&st, buf, 4), 6);
    CHECK_STR(buf, "mys");
    CHECK_INT(buf[4], 'Z');
    CHECK_INT(shell_state_format_prompt(&st, NULL, 0), 6);
}

/* ======================= builtin.c ======================= */

static void test_builtin_lookup_known_names(void)
{
    CHECK_INT(builtin_lookup("exit"), BUILTIN_EXIT);
    CHECK_INT(builtin_lookup("cd"), BUILTIN_CD);
    CHECK_INT(builtin_lookup("pwd"), BUILTIN_PWD);
    CHECK_INT(builtin_lookup("help"), BUILTIN_HELP);
    CHECK_INT(builtin_lookup("jobs"), BUILTIN_JOBS);
    CHECK_INT(builtin_lookup("history"), BUILTIN_HISTORY);
}

static void test_builtin_lookup_rejects_everything_else(void)
{
    CHECK_INT(builtin_lookup("ls"), BUILTIN_NONE);
    CHECK_INT(builtin_lookup("Exit"), BUILTIN_NONE);    /* case-sensitive */
    CHECK_INT(builtin_lookup("exit "), BUILTIN_NONE);   /* exact match only */
    CHECK_INT(builtin_lookup(" cd"), BUILTIN_NONE);
    CHECK_INT(builtin_lookup("ex"), BUILTIN_NONE);      /* prefix is not a match */
    CHECK_INT(builtin_lookup("cdd"), BUILTIN_NONE);
    CHECK_INT(builtin_lookup(""), BUILTIN_NONE);
    CHECK_INT(builtin_lookup(NULL), BUILTIN_NONE);
}

static void test_builtin_name_round_trip(void)
{
    int id;
    for (id = BUILTIN_EXIT; id <= BUILTIN_HISTORY; id++) {
        const char *name = builtin_name((enum builtin_id)id);
        CHECK(name != NULL);
        CHECK_INT(builtin_lookup(name), id);
    }
    CHECK_STR(builtin_name(BUILTIN_EXIT), "exit");
    CHECK_STR(builtin_name(BUILTIN_HISTORY), "history");
    CHECK(builtin_name(BUILTIN_NONE) == NULL);
    CHECK(builtin_name((enum builtin_id)99) == NULL);
}

/* ======================= input.c ======================= */

static void test_read_line_basic(void)
{
    FILE *f = tl_file_from_string("one\ntwo\n");
    char buf[32];
    CHECK_INT(read_line(f, buf, sizeof buf), 3);
    CHECK_STR(buf, "one");
    CHECK_INT(read_line(f, buf, sizeof buf), 3);
    CHECK_STR(buf, "two");
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_read_line_empty_input_is_eof(void)
{
    FILE *f = tl_file_from_string("");
    char buf[8];
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_read_line_last_line_without_newline(void)
{
    FILE *f = tl_file_from_string("first\nlast");
    char buf[32];
    CHECK_INT(read_line(f, buf, sizeof buf), 5);
    CHECK_STR(buf, "first");
    CHECK_INT(read_line(f, buf, sizeof buf), 4);
    CHECK_STR(buf, "last");
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_read_line_blank_lines(void)
{
    FILE *f = tl_file_from_string("\n\nx\n");
    char buf[8];
    CHECK_INT(read_line(f, buf, sizeof buf), 0);
    CHECK_STR(buf, "");
    CHECK_INT(read_line(f, buf, sizeof buf), 0);
    CHECK_INT(read_line(f, buf, sizeof buf), 1);
    CHECK_STR(buf, "x");
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_read_line_exact_fit(void)
{
    FILE *f = tl_file_from_string("1234567\nabc\n"); /* 7 chars fit in size 8 */
    char buf[8];
    CHECK_INT(read_line(f, buf, sizeof buf), 7);
    CHECK_STR(buf, "1234567");
    CHECK_INT(read_line(f, buf, sizeof buf), 3);
    CHECK_STR(buf, "abc");
    fclose(f);
}

static void test_read_line_too_long_discards_the_rest(void)
{
    FILE *f = tl_file_from_string("12345678\nok\n"); /* 8 chars do NOT fit in 8 */
    char buf[8];
    CHECK_INT(read_line(f, buf, sizeof buf), READ_TOO_LONG);
    CHECK_INT(read_line(f, buf, sizeof buf), 2);      /* next call is clean */
    CHECK_STR(buf, "ok");
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_read_line_very_long_line(void)
{
    char *big = malloc(20001);
    FILE *f;
    char buf[16];
    CHECK(big != NULL);
    memset(big, 'a', 20000);
    big[20000] = '\0';

    f = tmpfile();
    CHECK(f != NULL);
    fputs(big, f);
    fputs("\nafter\n", f);
    rewind(f);

    CHECK_INT(read_line(f, buf, sizeof buf), READ_TOO_LONG);
    CHECK_INT(read_line(f, buf, sizeof buf), 5);
    CHECK_STR(buf, "after");
    fclose(f);
    free(big);
}

static void test_read_line_too_long_at_end_of_file(void)
{
    FILE *f = tl_file_from_string("123456789"); /* no newline, too long for 8 */
    char buf[8];
    CHECK_INT(read_line(f, buf, sizeof buf), READ_TOO_LONG);
    CHECK_INT(read_line(f, buf, sizeof buf), READ_EOF);
    fclose(f);
}

static void test_str_trim(void)
{
    char a[] = "  hi  ";
    char b[] = "hi";
    char c[] = "\t a b \n";
    char d[] = "   ";
    char e[] = "";
    char f[] = "x   y";
    char *r;

    r = str_trim(a);
    CHECK_STR(r, "hi");
    CHECK_INT(r[2], 0);
    CHECK(r == a + 2);              /* returns a pointer INTO the buffer */
    CHECK_STR(str_trim(b), "hi");
    CHECK_STR(str_trim(c), "a b");
    CHECK_STR(str_trim(d), "");
    CHECK_STR(str_trim(e), "");
    CHECK_STR(str_trim(f), "x   y"); /* interior whitespace is kept */
    CHECK(str_trim(NULL) == NULL);
}

int main(void)
{
    tl_banner("Milestone 1 (Week 1): data model, state, built-ins, input");

    printf("command.c\n");
    RUN(test_command_init_clears_everything);
    RUN(test_add_arg_basic);
    RUN(test_add_arg_copies_the_string);
    RUN(test_add_arg_empty_string_is_a_word);
    RUN(test_add_arg_null_is_rejected);
    RUN(test_add_arg_full_command);
    RUN(test_add_arg_length_limit);
    RUN(test_set_infile);
    RUN(test_set_outfile_modes);
    RUN(test_set_outfile_failure_changes_nothing);
    RUN(test_argv_ptrs);
    RUN(test_argv_ptrs_empty_command);
    RUN(test_argv_ptrs_full_command);
    RUN(test_format_words_and_redirects);
    RUN(test_format_empty_command);
    RUN(test_format_truncates_like_snprintf);
    RUN(test_format_exact_fit_boundary);
    RUN(test_format_truncation_inside_redirect);
    RUN(test_pipeline_init);

    printf("shell_state.c\n");
    RUN(test_state_init);
    RUN(test_state_set_status_keeps_low_8_bits);
    RUN(test_prompt_success);
    RUN(test_prompt_shows_nonzero_status);
    RUN(test_prompt_truncates_like_snprintf);

    printf("builtin.c\n");
    RUN(test_builtin_lookup_known_names);
    RUN(test_builtin_lookup_rejects_everything_else);
    RUN(test_builtin_name_round_trip);

    printf("input.c\n");
    RUN(test_read_line_basic);
    RUN(test_read_line_empty_input_is_eof);
    RUN(test_read_line_last_line_without_newline);
    RUN(test_read_line_blank_lines);
    RUN(test_read_line_exact_fit);
    RUN(test_read_line_too_long_discards_the_rest);
    RUN(test_read_line_very_long_line);
    RUN(test_read_line_too_long_at_end_of_file);
    RUN(test_str_trim);

    return tl_summary();
}
