/* test_m2.c - unit tests for Milestone 2 (Week 2): mystring, strvec, lexer,
 * parser, joblist. Run with:  make test2
 *
 * Milestone 1 (command.c) must already work: the parser is built on it. */
#include "testlib.h"

#include "command.h"
#include "joblist.h"
#include "lexer.h"
#include "mystring.h"
#include "parser.h"
#include "strvec.h"

static int sgn(int x) { return (x > 0) - (x < 0); }

#define END ((const char *)NULL)

/* ======================= mystring.c ======================= */

static void test_strlen(void)
{
    static const char *cases[] = { "", "a", "hello", "hello, world: a longer one",
                                   "tab\tand\nnewline" };
    char big[1001];
    size_t i;
    for (i = 0; i < sizeof cases / sizeof cases[0]; i++)
        CHECK_INT(my_strlen(cases[i]), strlen(cases[i]));
    memset(big, 'x', 1000);
    big[1000] = '\0';
    CHECK_INT(my_strlen(big), 1000);
    CHECK_INT(my_strlen("abc\0def"), 3); /* stops at the first '\0' */
}

static void test_strcmp_matches_libc(void)
{
    static const char *a[] = { "", "", "a", "a", "b", "abc", "abcd", "abc", "abc",
                               "ABC", "apple", "app", "same", "\xff", "a" };
    static const char *b[] = { "", "a", "", "a", "a", "abcd", "abc", "abd", "abc",
                               "abc", "apple", "apple", "same", "a", "\xff" };
    size_t i;
    for (i = 0; i < sizeof a / sizeof a[0]; i++) {
        if (sgn(my_strcmp(a[i], b[i])) != sgn(strcmp(a[i], b[i]))) {
            fprintf(stderr, "         my_strcmp(\"%s\", \"%s\") has the wrong sign\n",
                    a[i], b[i]);
            _exit(1);
        }
    }
}

static void test_strcmp_compares_unsigned_chars(void)
{
    CHECK(my_strcmp("\xff", "a") > 0);   /* 255 > 97, even though (char)0xFF < 0 */
    CHECK(my_strcmp("a", "\xff") < 0);
    CHECK(my_strcmp("caf\xc3\xa9", "cafe") > 0);
}

static void test_strncmp_matches_libc(void)
{
    struct { const char *a, *b; size_t n; } t[] = {
        { "abcdef", "abcxyz", 3 },  { "abcdef", "abcxyz", 4 },
        { "abc", "abcd", 3 },       { "abc", "abcd", 4 },
        { "abc", "abd", 0 },        { "", "", 5 },
        { "a", "b", 100 },          { "abc", "abc", 1000000 },
        { "abcd", "abc", 4 },       { "\xff", "a", 1 },
    };
    size_t i;
    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        if (sgn(my_strncmp(t[i].a, t[i].b, t[i].n)) !=
            sgn(strncmp(t[i].a, t[i].b, t[i].n))) {
            fprintf(stderr, "         my_strncmp(\"%s\", \"%s\", %zu) is wrong\n",
                    t[i].a, t[i].b, t[i].n);
            _exit(1);
        }
    }
}

static void test_strcpy(void)
{
    char dst[16];
    memset(dst, 'Z', sizeof dst);
    CHECK(my_strcpy(dst, "hello") == dst);         /* returns dst */
    CHECK_STR(dst, "hello");
    CHECK_INT(dst[5], 0);
    CHECK_INT(dst[6], 'Z');                        /* nothing written after '\0' */

    memset(dst, 'Z', sizeof dst);
    my_strcpy(dst, "");
    CHECK_INT(dst[0], 0);
    CHECK_INT(dst[1], 'Z');
}

static void test_strncpy_pads_with_nuls(void)
{
    char dst[12];
    memset(dst, 'Z', sizeof dst);
    CHECK(my_strncpy(dst, "hi", 6) == dst);
    CHECK_INT(memcmp(dst, "hi\0\0\0\0", 6), 0);    /* padded up to n bytes */
    CHECK_INT(dst[6], 'Z');                        /* but never beyond n   */
}

static void test_strncpy_truncates_without_terminating(void)
{
    char dst[12];
    memset(dst, 'Z', sizeof dst);
    my_strncpy(dst, "abcdefgh", 4);
    CHECK_INT(memcmp(dst, "abcd", 4), 0);
    CHECK_INT(dst[4], 'Z');                        /* no '\0' was written */
}

static void test_strncpy_exact_length_and_zero(void)
{
    char dst[8];
    memset(dst, 'Z', sizeof dst);
    my_strncpy(dst, "abc", 3);
    CHECK_INT(memcmp(dst, "abc", 3), 0);
    CHECK_INT(dst[3], 'Z');                        /* src had exactly n chars: no '\0' */

    memset(dst, 'Z', sizeof dst);
    my_strncpy(dst, "abc", 0);
    CHECK_INT(dst[0], 'Z');                        /* n == 0 writes nothing */
}

static void test_strchr(void)
{
    const char *s = "hello world";
    CHECK(my_strchr(s, 'h') == s);
    CHECK(my_strchr(s, 'o') == s + 4);             /* the FIRST one */
    CHECK(my_strchr(s, 'd') == s + 10);
    CHECK(my_strchr(s, 'z') == NULL);
    CHECK(my_strchr(s, '\0') == s + 11);           /* the terminator counts */
    CHECK(my_strchr("", '\0') != NULL);
    CHECK(my_strchr("", 'a') == NULL);
    CHECK(my_strchr(s, 'o' + 256) == s + 4);       /* c is converted to char */
}

static void test_strdup(void)
{
    char src[] = "hello";
    char *d = my_strdup(src);
    CHECK(d != NULL);
    CHECK(d != src);
    CHECK_STR(d, "hello");
    src[0] = 'J';
    CHECK_STR(d, "hello");                         /* an independent copy */
    free(d);

    d = my_strdup("");
    CHECK(d != NULL);
    CHECK_STR(d, "");
    free(d);

    CHECK(my_strdup(NULL) == NULL);
}

static void test_strdup_out_of_memory(void)
{
    char *d;
    tl_alloc_fail_after(0);
    d = my_strdup("hello");
    tl_alloc_reset();
    CHECK(d == NULL);
}

/* ======================= strvec.c ======================= */

static void test_strvec_init(void)
{
    struct strvec v;
    memset(&v, 0xFF, sizeof v);
    strvec_init(&v);
    CHECK(v.items == NULL);
    CHECK_INT(v.len, 0);
    CHECK_INT(v.cap, 0);
}

static void test_strvec_push_and_get(void)
{
    struct strvec v;
    strvec_init(&v);
    CHECK_INT(strvec_push(&v, "alpha"), 0);
    CHECK_INT(strvec_push(&v, "beta"), 0);
    CHECK_INT(strvec_push(&v, "gamma"), 0);
    CHECK_INT(v.len, 3);
    CHECK_STR(strvec_get(&v, 0), "alpha");
    CHECK_STR(strvec_get(&v, 1), "beta");
    CHECK_STR(strvec_get(&v, 2), "gamma");
    CHECK(strvec_get(&v, 3) == NULL);
    CHECK(strvec_get(&v, 1000) == NULL);
    strvec_free(&v);
}

static void test_strvec_push_stores_a_private_copy(void)
{
    struct strvec v;
    char word[] = "first";
    strvec_init(&v);
    CHECK_INT(strvec_push(&v, word), 0);
    word[0] = 'X';
    CHECK_STR(strvec_get(&v, 0), "first");
    CHECK(strvec_get(&v, 0) != word);
    strvec_free(&v);
}

static void test_strvec_push_empty_string_and_null(void)
{
    struct strvec v;
    strvec_init(&v);
    CHECK_INT(strvec_push(&v, ""), 0);             /* "" is a perfectly good string */
    CHECK_INT(v.len, 1);
    CHECK_STR(strvec_get(&v, 0), "");
    CHECK_INT(strvec_push(&v, NULL), -1);
    CHECK_INT(v.len, 1);
    strvec_free(&v);
}

static void test_strvec_capacity_doubles(void)
{
    struct strvec v;
    size_t expected;
    int i;
    strvec_init(&v);
    for (i = 1; i <= 33; i++) {
        CHECK_INT(strvec_push(&v, "x"), 0);
        expected = i <= 4 ? 4 : i <= 8 ? 8 : i <= 16 ? 16 : i <= 32 ? 32 : 64;
        CHECK_INT(v.cap, expected);
    }
    strvec_free(&v);
}

static void test_strvec_many_items_survive_growth(void)
{
    struct strvec v;
    char buf[32];
    int i;
    strvec_init(&v);
    for (i = 0; i < 1000; i++) {
        snprintf(buf, sizeof buf, "item%d", i);
        CHECK_INT(strvec_push(&v, buf), 0);
    }
    CHECK_INT(v.len, 1000);
    for (i = 0; i < 1000; i++) {
        snprintf(buf, sizeof buf, "item%d", i);
        CHECK_STR(strvec_get(&v, (size_t)i), buf);
    }
    strvec_free(&v);
}

static void test_strvec_pop(void)
{
    struct strvec v;
    char *p;
    strvec_init(&v);
    CHECK(strvec_pop(&v) == NULL);                 /* popping an empty vector */
    strvec_push(&v, "a");
    strvec_push(&v, "b");
    strvec_push(&v, "c");

    p = strvec_pop(&v);
    CHECK_STR(p, "c");
    free(p);                                       /* the caller owns it now */
    CHECK_INT(v.len, 2);
    CHECK(strvec_get(&v, 2) == NULL);

    p = strvec_pop(&v); CHECK_STR(p, "b"); free(p);
    p = strvec_pop(&v); CHECK_STR(p, "a"); free(p);
    CHECK(strvec_pop(&v) == NULL);
    CHECK_INT(v.len, 0);
    CHECK_INT(v.cap, 4);                           /* capacity is kept */

    CHECK_INT(strvec_push(&v, "again"), 0);        /* still usable */
    CHECK_STR(strvec_get(&v, 0), "again");
    strvec_free(&v);
}

static void test_strvec_clear_keeps_capacity(void)
{
    struct strvec v;
    int i;
    strvec_init(&v);
    for (i = 0; i < 6; i++) strvec_push(&v, "x");
    CHECK_INT(v.cap, 8);
    strvec_clear(&v);
    CHECK_INT(v.len, 0);
    CHECK_INT(v.cap, 8);
    CHECK(v.items != NULL);
    CHECK(strvec_get(&v, 0) == NULL);
    CHECK_INT(strvec_push(&v, "new"), 0);
    CHECK_STR(strvec_get(&v, 0), "new");
    CHECK_INT(v.cap, 8);
    strvec_free(&v);
}

static void test_strvec_free_resets_and_is_idempotent(void)
{
    struct strvec v;
    strvec_init(&v);
    strvec_push(&v, "one");
    strvec_push(&v, "two");
    strvec_free(&v);
    CHECK(v.items == NULL);
    CHECK_INT(v.len, 0);
    CHECK_INT(v.cap, 0);
    strvec_free(&v);                               /* second call: must not crash */
    CHECK_INT(strvec_push(&v, "reborn"), 0);       /* and the vector is reusable */
    CHECK_STR(strvec_get(&v, 0), "reborn");
    strvec_free(&v);
}

static void test_strvec_first_push_survives_alloc_failure(void)
{
    int n;
    for (n = 0; n < 3; n++) {
        struct strvec v;
        int rc;
        strvec_init(&v);
        tl_alloc_fail_after(n);
        rc = strvec_push(&v, "x");
        tl_alloc_reset();
        if (n < 2) CHECK_INT(rc, -1);              /* needs 2 allocations: must fail */
        if (rc == -1) {
            CHECK_INT(v.len, 0);
        } else {
            CHECK_INT(v.len, 1);
            CHECK_STR(strvec_get(&v, 0), "x");
        }
        CHECK_INT(strvec_push(&v, "y"), 0);        /* still usable afterwards */
        CHECK_STR(strvec_get(&v, v.len - 1), "y");
        strvec_free(&v);
    }
}

static void test_strvec_growth_failure_keeps_old_items(void)
{
    struct strvec v;
    int n;
    strvec_init(&v);
    strvec_push(&v, "a");
    strvec_push(&v, "b");
    strvec_push(&v, "c");
    strvec_push(&v, "d");                          /* full: the next push must grow */
    for (n = 0; n < 2; n++) {
        tl_alloc_fail_after(n);
        CHECK_INT(strvec_push(&v, "e"), -1);
        tl_alloc_reset();
        CHECK_INT(v.len, 4);
        CHECK_STR(strvec_get(&v, 0), "a");         /* old contents still intact */
        CHECK_STR(strvec_get(&v, 3), "d");
    }
    CHECK_INT(strvec_push(&v, "e"), 0);
    CHECK_INT(v.len, 5);
    CHECK_STR(strvec_get(&v, 0), "a");
    CHECK_STR(strvec_get(&v, 4), "e");
    strvec_free(&v);
}

/* ======================= lexer.c ======================= */

/* Tokenize line and check that the result is exactly the n expected tokens. */
static void check_lex(const char *line, int n, ...)
{
    struct strvec v;
    va_list ap;
    int i, rc;

    strvec_init(&v);
    rc = tokenize(line, &v);
    if (rc != n || (int)v.len != n) {
        fprintf(stderr, "         tokenize(\"%s\"): expected %d tokens, got "
                "return value %d and len %d\n", line, n, rc, (int)v.len);
        _exit(1);
    }
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        const char *want = va_arg(ap, const char *);
        const char *got = strvec_get(&v, (size_t)i);
        if (got == NULL || strcmp(got, want) != 0) {
            fprintf(stderr, "         tokenize(\"%s\"): token %d should be \"%s\" "
                    "but is \"%s\"\n", line, i, want, got ? got : "(null)");
            _exit(1);
        }
    }
    va_end(ap);
    strvec_free(&v);
}

/* tokenize must fail with err and leave the vector empty. */
static void check_lex_error(const char *line, int err)
{
    struct strvec v;
    int rc;
    strvec_init(&v);
    rc = tokenize(line, &v);
    if (rc != err || v.len != 0) {
        fprintf(stderr, "         tokenize(\"%s\"): expected error %d with an empty "
                "vector, got return value %d and len %d\n", line, err, rc, (int)v.len);
        _exit(1);
    }
    strvec_free(&v);
}

static void test_lex_empty_and_blank_lines(void)
{
    check_lex("", 0);
    check_lex("   \t  ", 0);
    check_lex("\n", 0);
}

static void test_lex_plain_words(void)
{
    check_lex("ls", 1, "ls");
    check_lex("ls -l /tmp", 3, "ls", "-l", "/tmp");
    check_lex("  ls   -l\t/tmp \n", 3, "ls", "-l", "/tmp");
}

static void test_lex_operators_need_no_spaces(void)
{
    check_lex("ls>out", 3, "ls", ">", "out");
    check_lex("a|b", 3, "a", "|", "b");
    check_lex("a|b|c", 5, "a", "|", "b", "|", "c");
    check_lex("cat<in>out", 5, "cat", "<", "in", ">", "out");
    check_lex("sleep 5&", 3, "sleep", "5", "&");
    check_lex("a | b > c < d & ", 8, "a", "|", "b", ">", "c", "<", "d", "&");
}

static void test_lex_double_greater_is_one_token(void)
{
    check_lex("a>>b", 3, "a", ">>", "b");
    check_lex("a >> b", 3, "a", ">>", "b");
    check_lex("a>>>b", 4, "a", ">>", ">", "b");
    check_lex("a > > b", 4, "a", ">", ">", "b");
}

static void test_lex_ampersands_and_pipes_stay_single(void)
{
    check_lex("&", 1, "&");
    check_lex("a&&b", 4, "a", "&", "&", "b");
    check_lex("||", 2, "|", "|");
}

static void test_lex_double_quotes(void)
{
    check_lex("echo \"hello world\"", 2, "echo", "hello world");
    check_lex("\"  padded  \"", 1, "  padded  ");
}

static void test_lex_single_quotes(void)
{
    check_lex("echo 'hello world'", 2, "echo", "hello world");
}

static void test_lex_quotes_join_neighbouring_text(void)
{
    check_lex("he\"llo wo\"rld", 1, "hello world");
    check_lex("\"a b\"c'd e'", 1, "a bcd e");
    check_lex("x\"y\"z", 1, "xyz");
}

static void test_lex_other_quote_kind_is_plain_text(void)
{
    check_lex("\"it's\"", 1, "it's");
    check_lex("'say \"hi\"'", 1, "say \"hi\"");
}

static void test_lex_empty_quotes_make_empty_tokens(void)
{
    check_lex("echo \"\"", 2, "echo", "");
    check_lex("'' ''", 2, "", "");
    check_lex("a\"\"", 1, "a");
}

static void test_lex_operators_inside_quotes_are_text(void)
{
    check_lex("echo \"a|b\" '>' \"x y>z\"", 4, "echo", "a|b", ">", "x y>z");
}

static void test_lex_backslash_is_an_ordinary_character(void)
{
    check_lex("a\\b c\\", 2, "a\\b", "c\\");
}

static void test_lex_unterminated_quotes(void)
{
    check_lex_error("echo \"hi", LEX_ERR_QUOTE);
    check_lex_error("'abc", LEX_ERR_QUOTE);
    check_lex_error("\"a\" \"b", LEX_ERR_QUOTE);
    check_lex_error("a b \"c", LEX_ERR_QUOTE);     /* earlier tokens are discarded too */
    check_lex_error("it's", LEX_ERR_QUOTE);
}

static void test_lex_long_inputs(void)
{
    struct strvec v;
    char *line = malloc(20001);
    char *word = malloc(5001);
    int i;
    CHECK(line != NULL && word != NULL);

    for (i = 0; i < 10000; i++) {                  /* "w w w w ... w" */
        line[2 * i] = 'w';
        line[2 * i + 1] = ' ';
    }
    line[20000] = '\0';
    strvec_init(&v);
    CHECK_INT(tokenize(line, &v), 10000);
    CHECK_INT(v.len, 10000);
    CHECK_STR(strvec_get(&v, 9999), "w");
    strvec_free(&v);

    memset(word, 'k', 5000);
    word[5000] = '\0';
    strvec_init(&v);
    CHECK_INT(tokenize(word, &v), 1);              /* one huge token */
    CHECK_STR(strvec_get(&v, 0), word);
    strvec_free(&v);

    free(line);
    free(word);
}

static void test_lex_survives_alloc_failure(void)
{
    int n, saw_failure = 0, saw_success = 0;
    for (n = 0; n < 24; n++) {
        struct strvec v;
        int rc;
        strvec_init(&v);
        tl_alloc_fail_after(n);
        rc = tokenize("ls -l \"a b\" | wc > out &", &v);
        tl_alloc_reset();
        if (rc >= 0) {
            CHECK_INT(rc, 8);
            CHECK_INT(v.len, 8);
            saw_success = 1;
        } else {
            CHECK_INT(rc, LEX_ERR_NOMEM);          /* the right error code... */
            CHECK_INT(v.len, 0);                   /* ...and nothing half-done */
            saw_failure = 1;
        }
        strvec_free(&v);                           /* ASan/valgrind catch leaks here */
    }
    CHECK(saw_failure);                            /* a failing malloc must be noticed */
    CHECK(saw_success);
}

/* ======================= parser.c ======================= */

/* Build a token vector from a NULL-terminated list of strings, parse it into
 * *pl and return parse_pipeline's result. Use it through the PARSE macro. */
static int parse_list(struct pipeline *pl, ...)
{
    struct strvec v;
    va_list ap;
    const char *s;
    int rc;

    strvec_init(&v);
    va_start(ap, pl);
    while ((s = va_arg(ap, const char *)) != NULL) CHECK_INT(strvec_push(&v, s), 0);
    va_end(ap);
    rc = parse_pipeline(&v, pl);
    strvec_free(&v);
    return rc;
}

#define PARSE(pl, ...) parse_list((pl), __VA_ARGS__, END)

/* Parse the given token list and require a specific return code. */
#define EXPECT_CODE(code, ...)                                                  \
    do {                                                                        \
        struct pipeline pl_;                                                    \
        int rc_ = PARSE(&pl_, __VA_ARGS__);                                     \
        if (rc_ != (code)) {                                                    \
            fprintf(stderr, "         %s:%d: expected return code %d for this "  \
                    "token list, got %d\n", __FILE__, __LINE__, (code), rc_);   \
            _exit(1);                                                           \
        }                                                                       \
    } while (0)

static void test_parse_empty_token_list(void)
{
    struct pipeline pl;
    struct strvec v;
    memset(&pl, 0xFF, sizeof pl);
    strvec_init(&v);
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_EMPTY);
    CHECK_INT(pl.ncmds, 0);
    CHECK_INT(pl.background, 0);
    strvec_free(&v);
}

static void test_parse_simple_command(void)
{
    struct pipeline pl;
    CHECK_INT(PARSE(&pl, "ls", "-l"), PARSE_OK);
    CHECK_INT(pl.ncmds, 1);
    CHECK_INT(pl.background, 0);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[0], "ls");
    CHECK_STR(pl.cmds[0].argv[1], "-l");
    CHECK_STR(pl.cmds[0].infile, "");
    CHECK_STR(pl.cmds[0].outfile, "");
    CHECK_INT(pl.cmds[0].append, 0);
}

static void test_parse_redirections(void)
{
    struct pipeline pl;

    CHECK_INT(PARSE(&pl, "cat", "<", "in.txt"), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 1);
    CHECK_STR(pl.cmds[0].infile, "in.txt");
    CHECK_STR(pl.cmds[0].outfile, "");

    CHECK_INT(PARSE(&pl, "cat", ">", "out.txt"), PARSE_OK);
    CHECK_STR(pl.cmds[0].outfile, "out.txt");
    CHECK_INT(pl.cmds[0].append, 0);

    CHECK_INT(PARSE(&pl, "cat", ">>", "log.txt"), PARSE_OK);
    CHECK_STR(pl.cmds[0].outfile, "log.txt");
    CHECK_INT(pl.cmds[0].append, 1);

    CHECK_INT(PARSE(&pl, "cat", "<", "a", ">", "b"), PARSE_OK);
    CHECK_STR(pl.cmds[0].infile, "a");
    CHECK_STR(pl.cmds[0].outfile, "b");
    CHECK_INT(pl.cmds[0].argc, 1);
}

static void test_parse_redirections_can_appear_anywhere(void)
{
    struct pipeline pl;

    CHECK_INT(PARSE(&pl, ">", "out", "ls", "-l"), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[0], "ls");
    CHECK_STR(pl.cmds[0].argv[1], "-l");
    CHECK_STR(pl.cmds[0].outfile, "out");

    CHECK_INT(PARSE(&pl, "ls", ">", "out", "-l"), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[1], "-l");
    CHECK_STR(pl.cmds[0].outfile, "out");

    CHECK_INT(PARSE(&pl, "<", "in", "cat"), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 1);
    CHECK_STR(pl.cmds[0].infile, "in");
}

static void test_parse_two_command_pipeline(void)
{
    struct pipeline pl;
    CHECK_INT(PARSE(&pl, "ls", "-l", "|", "wc", "-l"), PARSE_OK);
    CHECK_INT(pl.ncmds, 2);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[0], "ls");
    CHECK_STR(pl.cmds[0].argv[1], "-l");
    CHECK_INT(pl.cmds[1].argc, 2);
    CHECK_STR(pl.cmds[1].argv[0], "wc");
    CHECK_STR(pl.cmds[1].argv[1], "-l");
}

static void test_parse_redirects_at_the_ends_of_a_pipeline(void)
{
    struct pipeline pl;
    CHECK_INT(PARSE(&pl, "cat", "<", "in", "|", "sort", "|", "uniq", ">", "out"), PARSE_OK);
    CHECK_INT(pl.ncmds, 3);
    CHECK_STR(pl.cmds[0].infile, "in");
    CHECK_STR(pl.cmds[0].outfile, "");
    CHECK_STR(pl.cmds[1].infile, "");
    CHECK_STR(pl.cmds[1].outfile, "");
    CHECK_STR(pl.cmds[2].infile, "");
    CHECK_STR(pl.cmds[2].outfile, "out");
    CHECK_STR(pl.cmds[1].argv[0], "sort");
}

static void test_parse_background(void)
{
    struct pipeline pl;
    CHECK_INT(PARSE(&pl, "sleep", "5", "&"), PARSE_OK);
    CHECK_INT(pl.background, 1);
    CHECK_INT(pl.cmds[0].argc, 2);                 /* the '&' is not an argument */

    CHECK_INT(PARSE(&pl, "ls", "|", "wc", "&"), PARSE_OK);
    CHECK_INT(pl.background, 1);
    CHECK_INT(pl.ncmds, 2);

    CHECK_INT(PARSE(&pl, "ls", ">", "out", "&"), PARSE_OK);
    CHECK_INT(pl.background, 1);

    CHECK_INT(PARSE(&pl, "sleep", "5"), PARSE_OK);
    CHECK_INT(pl.background, 0);
}

static void test_parse_pipe_syntax_errors(void)
{
    EXPECT_CODE(PARSE_ERR_SYNTAX, "|");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "|");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "|", "ls");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "|", "|", "wc");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "a", "|", "b", "|");
}

static void test_parse_redirect_syntax_errors(void)
{
    EXPECT_CODE(PARSE_ERR_SYNTAX, "<");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "<");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">>");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "<", ">", "out");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">", "|", "wc");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">", "&");
    EXPECT_CODE(PARSE_ERR_SYNTAX, ">", "out");           /* no command word at all */
    EXPECT_CODE(PARSE_ERR_SYNTAX, "<", "in");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "|", ">", "out"); /* 2nd command has no word */
}

static void test_parse_ampersand_syntax_errors(void)
{
    EXPECT_CODE(PARSE_ERR_SYNTAX, "&");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "&", "ls");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "&", "ls");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "&", "&");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "|", "&");
}

static void test_parse_redirect_placement_rules(void)
{
    /* "<" only on the first command, ">" / ">>" only on the last. */
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", "|", "cat", "<", "in");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">", "out", "|", "cat");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "ls", ">>", "out", "|", "cat");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "a", "|", "b", "<", "in", "|", "c");
    EXPECT_CODE(PARSE_OK, "a", "<", "in", "|", "b", "|", "c", ">", "out");
}

static void test_parse_duplicate_redirects_are_errors(void)
{
    EXPECT_CODE(PARSE_ERR_SYNTAX, "cat", "<", "a", "<", "b");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "cat", ">", "a", ">", "b");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "cat", ">", "a", ">>", "b");
    EXPECT_CODE(PARSE_OK, "cat", "<", "a", ">", "b");    /* one of each is fine */
}

static void test_parse_empty_word_vs_empty_filename(void)
{
    struct pipeline pl;
    CHECK_INT(PARSE(&pl, "echo", ""), PARSE_OK);         /* "" is a valid word */
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[1], "");
    EXPECT_CODE(PARSE_ERR_SYNTAX, "cat", ">", "");       /* but not a file name */
    EXPECT_CODE(PARSE_ERR_SYNTAX, "cat", "<", "");
}

static void test_parse_argument_limit(void)
{
    struct strvec v;
    struct pipeline pl;
    int i;
    strvec_init(&v);
    for (i = 0; i < MAX_ARGS; i++) CHECK_INT(strvec_push(&v, "w"), 0);
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, MAX_ARGS);
    CHECK_INT(strvec_push(&v, "w"), 0);
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_ERR_TOO_MANY_ARGS);
    strvec_free(&v);
}

static void test_parse_command_limit(void)
{
    struct strvec v;
    struct pipeline pl;
    int i;
    strvec_init(&v);
    for (i = 0; i < MAX_CMDS; i++) {
        if (i > 0) CHECK_INT(strvec_push(&v, "|"), 0);
        CHECK_INT(strvec_push(&v, "c"), 0);
    }
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_OK);
    CHECK_INT(pl.ncmds, MAX_CMDS);
    CHECK_INT(strvec_push(&v, "|"), 0);
    CHECK_INT(strvec_push(&v, "c"), 0);
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_ERR_TOO_MANY_CMDS);
    strvec_free(&v);
}

static void test_parse_length_limits(void)
{
    struct pipeline pl;
    char word[MAX_ARG_LEN + 2];
    char path[MAX_PATH_LEN + 2];

    memset(word, 'w', MAX_ARG_LEN);
    word[MAX_ARG_LEN] = '\0';
    CHECK_INT(PARSE(&pl, word), PARSE_OK);                 /* exactly the maximum */
    CHECK_STR(pl.cmds[0].argv[0], word);
    word[MAX_ARG_LEN] = 'w';
    word[MAX_ARG_LEN + 1] = '\0';
    CHECK_INT(PARSE(&pl, word), PARSE_ERR_TOO_LONG);       /* one more */

    memset(path, 'p', MAX_PATH_LEN);
    path[MAX_PATH_LEN] = '\0';
    CHECK_INT(PARSE(&pl, "cat", ">", path), PARSE_OK);
    CHECK_STR(pl.cmds[0].outfile, path);
    CHECK_INT(PARSE(&pl, "cat", "<", path), PARSE_OK);
    path[MAX_PATH_LEN] = 'p';
    path[MAX_PATH_LEN + 1] = '\0';
    CHECK_INT(PARSE(&pl, "cat", ">", path), PARSE_ERR_TOO_LONG);
    CHECK_INT(PARSE(&pl, "cat", ">>", path), PARSE_ERR_TOO_LONG);
    CHECK_INT(PARSE(&pl, "cat", "<", path), PARSE_ERR_TOO_LONG);
}

static void test_parse_starts_from_a_clean_pipeline(void)
{
    struct pipeline pl;
    struct strvec v;

    memset(&pl, 0xFF, sizeof pl);                  /* garbage in... */
    CHECK_INT(PARSE(&pl, "ls"), PARSE_OK);
    CHECK_INT(pl.ncmds, 1);                        /* ...must not leak out */
    CHECK_INT(pl.background, 0);
    CHECK_INT(pl.cmds[0].argc, 1);
    CHECK_INT(pl.cmds[0].append, 0);
    CHECK_STR(pl.cmds[0].infile, "");
    CHECK_INT(pl.cmds[5].argc, 0);

    memset(&pl, 0xFF, sizeof pl);
    strvec_init(&v);
    CHECK_INT(parse_pipeline(&v, &pl), PARSE_EMPTY);
    CHECK_INT(pl.ncmds, 0);
    CHECK_INT(pl.background, 0);
    strvec_free(&v);
}

/* ----- lexer + parser together: the real front end of the shell ----- */

static int lex_and_parse(const char *line, struct pipeline *pl)
{
    struct strvec v;
    int rc;
    strvec_init(&v);
    CHECK(tokenize(line, &v) >= 0);
    rc = parse_pipeline(&v, pl);
    strvec_free(&v);
    return rc;
}

static void test_front_end_compact_line(void)
{
    struct pipeline pl;
    CHECK_INT(lex_and_parse("cat<in|sort>out&", &pl), PARSE_OK);
    CHECK_INT(pl.ncmds, 2);
    CHECK_INT(pl.background, 1);
    CHECK_STR(pl.cmds[0].argv[0], "cat");
    CHECK_STR(pl.cmds[0].infile, "in");
    CHECK_STR(pl.cmds[1].argv[0], "sort");
    CHECK_STR(pl.cmds[1].outfile, "out");
}

static void test_front_end_quoted_arguments(void)
{
    struct pipeline pl;
    CHECK_INT(lex_and_parse("echo \"hello   world\" > \"my file.txt\"", &pl), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].argv[1], "hello   world");
    CHECK_STR(pl.cmds[0].outfile, "my file.txt");
}

static void test_front_end_append_and_input(void)
{
    struct pipeline pl;
    CHECK_INT(lex_and_parse("sort -r<in>>log", &pl), PARSE_OK);
    CHECK_INT(pl.cmds[0].argc, 2);
    CHECK_STR(pl.cmds[0].infile, "in");
    CHECK_STR(pl.cmds[0].outfile, "log");
    CHECK_INT(pl.cmds[0].append, 1);
}

static void test_front_end_errors_and_blank_lines(void)
{
    struct pipeline pl;
    CHECK_INT(lex_and_parse("ls -l | | wc", &pl), PARSE_ERR_SYNTAX);
    CHECK_INT(lex_and_parse("   \t ", &pl), PARSE_EMPTY);
    CHECK_INT(lex_and_parse("ls &&", &pl), PARSE_ERR_SYNTAX);
}

/* ======================= joblist.c ======================= */

static void format_must_be(const struct joblist *jl, const char *expected)
{
    char buf[512];
    int n = joblist_format(jl, buf, sizeof buf);
    if (n != (int)strlen(expected) || strcmp(buf, expected) != 0) {
        fprintf(stderr, "         joblist_format: expected (%d chars) \"%s\"\n"
                        "                         got      (%d chars) \"%s\"\n",
                (int)strlen(expected), expected, n, buf);
        _exit(1);
    }
}

static void test_joblist_init(void)
{
    struct joblist jl;
    memset(&jl, 0xFF, sizeof jl);
    joblist_init(&jl);
    CHECK(jl.head == NULL);
    CHECK_INT(jl.next_id, 1);
    CHECK_INT(joblist_count(&jl), 0);
}

static void test_joblist_add_gives_sequential_ids(void)
{
    struct joblist jl;
    joblist_init(&jl);
    CHECK_INT(joblist_add(&jl, 100, "a"), 1);
    CHECK_INT(joblist_add(&jl, 200, "b"), 2);
    CHECK_INT(joblist_add(&jl, 300, "c"), 3);
    CHECK_INT(joblist_count(&jl), 3);
    joblist_free(&jl);
}

static void test_joblist_add_keeps_oldest_first_order(void)
{
    struct joblist jl;
    joblist_init(&jl);
    joblist_add(&jl, 100, "first");
    joblist_add(&jl, 200, "second");
    joblist_add(&jl, 300, "third");
    CHECK_INT(jl.head->pid, 100);                  /* head is the OLDEST job */
    CHECK_INT(jl.head->next->pid, 200);
    CHECK_INT(jl.head->next->next->pid, 300);
    CHECK(jl.head->next->next->next == NULL);
    joblist_free(&jl);
}

static void test_joblist_add_copies_the_command_text(void)
{
    struct joblist jl;
    char text[] = "sleep 10";
    struct job *j;
    joblist_init(&jl);
    CHECK_INT(joblist_add(&jl, 42, text), 1);
    text[0] = 'X';
    j = joblist_find_pid(&jl, 42);
    CHECK(j != NULL);
    CHECK_STR(j->cmdline, "sleep 10");
    CHECK(j->cmdline != text);
    joblist_free(&jl);
}

static void test_joblist_add_null_command(void)
{
    struct joblist jl;
    joblist_init(&jl);
    CHECK_INT(joblist_add(&jl, 1, NULL), -1);
    CHECK_INT(joblist_count(&jl), 0);
    CHECK_INT(joblist_add(&jl, 1, "ok"), 1);       /* the failed call used no id */
    joblist_free(&jl);
}

static void test_joblist_find_pid(void)
{
    struct joblist jl;
    struct job *j;
    joblist_init(&jl);
    CHECK(joblist_find_pid(&jl, 100) == NULL);     /* empty list */
    joblist_add(&jl, 100, "one");
    joblist_add(&jl, 200, "two");
    j = joblist_find_pid(&jl, 200);
    CHECK(j != NULL);
    CHECK_INT(j->id, 2);
    CHECK_INT(j->pid, 200);
    CHECK_STR(j->cmdline, "two");
    CHECK(joblist_find_pid(&jl, 999) == NULL);
    joblist_free(&jl);
}

static void test_joblist_remove_head_middle_tail(void)
{
    struct joblist jl;

    joblist_init(&jl);
    joblist_add(&jl, 100, "a"); joblist_add(&jl, 200, "b"); joblist_add(&jl, 300, "c");
    CHECK_INT(joblist_remove_pid(&jl, 100), 0);                  /* head */
    format_must_be(&jl, "[2] 200 b\n[3] 300 c\n");
    joblist_free(&jl);

    joblist_init(&jl);
    joblist_add(&jl, 100, "a"); joblist_add(&jl, 200, "b"); joblist_add(&jl, 300, "c");
    CHECK_INT(joblist_remove_pid(&jl, 200), 0);                  /* middle */
    format_must_be(&jl, "[1] 100 a\n[3] 300 c\n");
    joblist_free(&jl);

    joblist_init(&jl);
    joblist_add(&jl, 100, "a"); joblist_add(&jl, 200, "b"); joblist_add(&jl, 300, "c");
    CHECK_INT(joblist_remove_pid(&jl, 300), 0);                  /* tail */
    format_must_be(&jl, "[1] 100 a\n[2] 200 b\n");
    joblist_add(&jl, 400, "d");                                  /* append still works */
    format_must_be(&jl, "[1] 100 a\n[2] 200 b\n[4] 400 d\n");
    joblist_free(&jl);
}

static void test_joblist_remove_only_element_and_missing(void)
{
    struct joblist jl;
    joblist_init(&jl);
    joblist_add(&jl, 100, "a");
    CHECK_INT(joblist_remove_pid(&jl, 999), -1);                 /* not there */
    CHECK_INT(joblist_count(&jl), 1);
    CHECK_INT(joblist_remove_pid(&jl, 100), 0);
    CHECK(jl.head == NULL);
    CHECK_INT(joblist_count(&jl), 0);
    CHECK_INT(joblist_remove_pid(&jl, 100), -1);                 /* already gone */
    CHECK_INT(joblist_remove_pid(&jl, 100), -1);
    joblist_add(&jl, 500, "again");                              /* list is reusable */
    CHECK_INT(joblist_count(&jl), 1);
    joblist_free(&jl);
}

static void test_joblist_ids_are_not_reused(void)
{
    struct joblist jl;
    joblist_init(&jl);
    joblist_add(&jl, 100, "a");
    joblist_add(&jl, 200, "b");
    joblist_add(&jl, 300, "c");
    joblist_remove_pid(&jl, 300);
    CHECK_INT(joblist_add(&jl, 400, "d"), 4);      /* 3 is NOT handed out again */
    joblist_remove_pid(&jl, 100);
    CHECK_INT(joblist_add(&jl, 500, "e"), 5);
    joblist_free(&jl);
}

static void test_joblist_format(void)
{
    struct joblist jl;
    char buf[16] = "garbage";

    joblist_init(&jl);
    CHECK_INT(joblist_format(&jl, buf, sizeof buf), 0);          /* empty list */
    CHECK_STR(buf, "");
    CHECK_INT(joblist_format(&jl, NULL, 0), 0);

    joblist_add(&jl, 4021, "sleep 10");
    joblist_add(&jl, 4030, "ls | wc");
    format_must_be(&jl, "[1] 4021 sleep 10\n[2] 4030 ls | wc\n");
    joblist_free(&jl);
}

static void test_joblist_format_truncates_like_snprintf(void)
{
    struct joblist jl;
    const char *full = "[1] 4021 sleep 10\n[2] 4030 ls | wc\n";
    size_t len = strlen(full);
    char buf[128];
    size_t size;

    joblist_init(&jl);
    joblist_add(&jl, 4021, "sleep 10");
    joblist_add(&jl, 4030, "ls | wc");

    CHECK_INT(joblist_format(&jl, NULL, 0), len);                /* size 0: just measure */

    /* Every buffer size from 1 up to a bit past the full length: the return
     * value is always the FULL length, the buffer always holds a '\0'-terminated
     * prefix, and nothing is ever written beyond size bytes. */
    for (size = 1; size <= len + 3; size++) {
        size_t keep = (size - 1 < len) ? size - 1 : len;
        memset(buf, 'Z', sizeof buf);
        CHECK_INT(joblist_format(&jl, buf, size), len);
        CHECK_INT(memcmp(buf, full, keep), 0);
        CHECK_INT(buf[keep], 0);
        CHECK_INT(buf[size], 'Z');
    }
    joblist_free(&jl);
}

static void test_joblist_free(void)
{
    struct joblist jl;
    joblist_init(&jl);
    joblist_add(&jl, 1, "a");
    joblist_add(&jl, 2, "b");
    joblist_add(&jl, 3, "c");
    joblist_free(&jl);
    CHECK(jl.head == NULL);
    CHECK_INT(jl.next_id, 1);                      /* back to the initial state */
    CHECK_INT(joblist_count(&jl), 0);
    joblist_free(&jl);                             /* a second call is harmless */
    CHECK_INT(joblist_add(&jl, 9, "fresh"), 1);
    joblist_free(&jl);
}

static void test_joblist_many_jobs(void)
{
    struct joblist jl;
    int i;
    joblist_init(&jl);
    for (i = 1; i <= 1000; i++) CHECK_INT(joblist_add(&jl, i, "job"), i);
    CHECK_INT(joblist_count(&jl), 1000);
    for (i = 2; i <= 1000; i += 2) CHECK_INT(joblist_remove_pid(&jl, i), 0);
    CHECK_INT(joblist_count(&jl), 500);
    for (i = 1; i <= 1000; i++) {
        if (i % 2) CHECK(joblist_find_pid(&jl, i) != NULL);
        else CHECK(joblist_find_pid(&jl, i) == NULL);
    }
    joblist_free(&jl);
}

static void test_joblist_add_survives_alloc_failure(void)
{
    int n;
    for (n = 0; n < 3; n++) {
        struct joblist jl;
        int rc;
        joblist_init(&jl);
        tl_alloc_fail_after(n);
        rc = joblist_add(&jl, 7, "x");
        tl_alloc_reset();
        if (n < 2) CHECK_INT(rc, -1);              /* node + text: 2 allocations */
        if (rc == -1) {
            CHECK_INT(joblist_count(&jl), 0);
            CHECK(jl.head == NULL);
            CHECK_INT(joblist_add(&jl, 8, "y"), 1);   /* no id was consumed */
        } else {
            CHECK_INT(rc, 1);
            CHECK_INT(joblist_add(&jl, 8, "y"), 2);
        }
        joblist_free(&jl);                         /* leak checkers watch here */
    }
}

int main(void)
{
    tl_banner("Milestone 2 (Week 2): strings, dynamic memory, tokenizer, parser, jobs");

    printf("mystring.c\n");
    RUN(test_strlen);
    RUN(test_strcmp_matches_libc);
    RUN(test_strcmp_compares_unsigned_chars);
    RUN(test_strncmp_matches_libc);
    RUN(test_strcpy);
    RUN(test_strncpy_pads_with_nuls);
    RUN(test_strncpy_truncates_without_terminating);
    RUN(test_strncpy_exact_length_and_zero);
    RUN(test_strchr);
    RUN(test_strdup);
    RUN(test_strdup_out_of_memory);

    printf("strvec.c\n");
    RUN(test_strvec_init);
    RUN(test_strvec_push_and_get);
    RUN(test_strvec_push_stores_a_private_copy);
    RUN(test_strvec_push_empty_string_and_null);
    RUN(test_strvec_capacity_doubles);
    RUN(test_strvec_many_items_survive_growth);
    RUN(test_strvec_pop);
    RUN(test_strvec_clear_keeps_capacity);
    RUN(test_strvec_free_resets_and_is_idempotent);
    RUN(test_strvec_first_push_survives_alloc_failure);
    RUN(test_strvec_growth_failure_keeps_old_items);

    printf("lexer.c\n");
    RUN(test_lex_empty_and_blank_lines);
    RUN(test_lex_plain_words);
    RUN(test_lex_operators_need_no_spaces);
    RUN(test_lex_double_greater_is_one_token);
    RUN(test_lex_ampersands_and_pipes_stay_single);
    RUN(test_lex_double_quotes);
    RUN(test_lex_single_quotes);
    RUN(test_lex_quotes_join_neighbouring_text);
    RUN(test_lex_other_quote_kind_is_plain_text);
    RUN(test_lex_empty_quotes_make_empty_tokens);
    RUN(test_lex_operators_inside_quotes_are_text);
    RUN(test_lex_backslash_is_an_ordinary_character);
    RUN(test_lex_unterminated_quotes);
    RUN(test_lex_long_inputs);
    RUN(test_lex_survives_alloc_failure);

    printf("parser.c\n");
    RUN(test_parse_empty_token_list);
    RUN(test_parse_simple_command);
    RUN(test_parse_redirections);
    RUN(test_parse_redirections_can_appear_anywhere);
    RUN(test_parse_two_command_pipeline);
    RUN(test_parse_redirects_at_the_ends_of_a_pipeline);
    RUN(test_parse_background);
    RUN(test_parse_pipe_syntax_errors);
    RUN(test_parse_redirect_syntax_errors);
    RUN(test_parse_ampersand_syntax_errors);
    RUN(test_parse_redirect_placement_rules);
    RUN(test_parse_duplicate_redirects_are_errors);
    RUN(test_parse_empty_word_vs_empty_filename);
    RUN(test_parse_argument_limit);
    RUN(test_parse_command_limit);
    RUN(test_parse_length_limits);
    RUN(test_parse_starts_from_a_clean_pipeline);

    printf("lexer + parser together\n");
    RUN(test_front_end_compact_line);
    RUN(test_front_end_quoted_arguments);
    RUN(test_front_end_append_and_input);
    RUN(test_front_end_errors_and_blank_lines);

    printf("joblist.c\n");
    RUN(test_joblist_init);
    RUN(test_joblist_add_gives_sequential_ids);
    RUN(test_joblist_add_keeps_oldest_first_order);
    RUN(test_joblist_add_copies_the_command_text);
    RUN(test_joblist_add_null_command);
    RUN(test_joblist_find_pid);
    RUN(test_joblist_remove_head_middle_tail);
    RUN(test_joblist_remove_only_element_and_missing);
    RUN(test_joblist_ids_are_not_reused);
    RUN(test_joblist_format);
    RUN(test_joblist_format_truncates_like_snprintf);
    RUN(test_joblist_free);
    RUN(test_joblist_many_jobs);
    RUN(test_joblist_add_survives_alloc_failure);

    return tl_summary();
}
