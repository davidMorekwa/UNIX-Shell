/*
 * testlib.h - a tiny unit-test harness for the mysh milestones.
 *
 * You do NOT need to edit or fully understand this file.
 *
 * How it works:
 *   - Every test function runs in its own child process, so a crash (segfault)
 *     or an infinite loop in one test cannot hide the results of the others.
 *   - A test passes if it returns normally. A failed CHECK_* prints where and
 *     why, then ends that test.
 *   - Each test gets a fresh, empty temporary directory (tl_tmpdir).
 *   - A test that runs longer than TEST_TIMEOUT seconds (default 10) is killed
 *     and reported as a TIMEOUT.
 */
#ifndef TESTLIB_H
#define TESTLIB_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ---- allocation-failure injection (implemented in alloc_fail.c) ---------- */
/* Make the allocation that happens after n more successful ones fail, once.
 * n = 0 means "the very next malloc/calloc/realloc fails". */
void tl_alloc_fail_after(int n);
/* Turn failure injection off again. */
void tl_alloc_reset(void);

/* ---- state ---------------------------------------------------------------- */
static int tl_total = 0;
static int tl_passed = 0;
static char tl_tmpdir[64];

/* ---- assertions: on failure print a message and end the current test ------ */
#define TL_FAIL_EXIT() _exit(1)

#define CHECK(cond)                                                             \
    do {                                                                        \
        if (!(cond)) {                                                          \
            fprintf(stderr, "         %s:%d: CHECK failed: %s\n", __FILE__,     \
                    __LINE__, #cond);                                           \
            TL_FAIL_EXIT();                                                     \
        }                                                                       \
    } while (0)

#define CHECK_INT(actual, expected)                                             \
    do {                                                                        \
        long long a_ = (long long)(actual), e_ = (long long)(expected);         \
        if (a_ != e_) {                                                         \
            fprintf(stderr,                                                     \
                    "         %s:%d: %s\n"                                      \
                    "             expected %lld, got %lld\n",                   \
                    __FILE__, __LINE__, #actual, e_, a_);                       \
            TL_FAIL_EXIT();                                                     \
        }                                                                       \
    } while (0)

#define CHECK_STR(actual, expected)                                             \
    do {                                                                        \
        const char *a_ = (actual), *e_ = (expected);                            \
        if (a_ == NULL || strcmp(a_, e_) != 0) {                                \
            fprintf(stderr,                                                     \
                    "         %s:%d: %s\n"                                      \
                    "             expected \"%s\", got \"%s\"\n",               \
                    __FILE__, __LINE__, #actual, e_,                            \
                    a_ ? a_ : "(null)");                                        \
            TL_FAIL_EXIT();                                                     \
        }                                                                       \
    } while (0)

/* ---- running tests ---------------------------------------------------------- */
typedef void (*tl_fn)(void);

static void tl_remove_tmpdir(void)
{
    char cmd[128];
    snprintf(cmd, sizeof cmd, "rm -rf '%s'", tl_tmpdir);
    if (system(cmd) != 0) { /* nothing useful to do */ }
}

static void tl_run(const char *name, tl_fn fn)
{
    const char *t = getenv("TEST_TIMEOUT");
    unsigned timeout = t ? (unsigned)atoi(t) : 10;
    const char *only = getenv("TEST");   /* TEST=pipe runs only tests whose name contains "pipe" */
    int status = 0;
    pid_t pid;

    if (only != NULL && only[0] != '\0' && strstr(name, only) == NULL) return;
    tl_total++;
    snprintf(tl_tmpdir, sizeof tl_tmpdir, "/tmp/mysh_test_XXXXXX");
    if (mkdtemp(tl_tmpdir) == NULL) {
        perror("mkdtemp");
        exit(2);
    }

    fflush(NULL);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(2);
    }
    if (pid == 0) {
        setpgid(0, 0);
        alarm(timeout);
        fn();
        fflush(NULL);
        exit(0); /* a normal exit lets valgrind/ASan check for leaks */
    }
    setpgid(pid, pid);
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    kill(-pid, SIGKILL); /* clean up any helper processes the test left behind */
    tl_remove_tmpdir();

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        tl_passed++;
        printf("  [PASS] %s\n", name);
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
        printf("  [FAIL] %s\n", name);
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
        printf("  [FAIL] %s\n         TIMEOUT after %us (infinite loop? "
               "a pipe end that was never closed?)\n", name, timeout);
    } else if (WIFSIGNALED(status)) {
        printf("  [FAIL] %s\n         CRASHED: %s\n", name,
               strsignal(WTERMSIG(status)));
    } else {
        printf("  [FAIL] %s\n         exit code %d (a memory error or leak was "
               "reported above by ASan/valgrind?)\n", name,
               WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    }
    fflush(stdout);
}

#define RUN(fn) tl_run(#fn, fn)

static void tl_banner(const char *title)
{
    int fd;
    /* Make sure descriptors 0, 1 and 2 are open, so that "the lowest free
     * descriptor" means the same thing however the tests were started. */
    for (fd = 0; fd <= 2; fd++) {
        if (fcntl(fd, F_GETFD) == -1) {
            int n = open("/dev/null", O_RDWR);
            (void)n;
        }
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("== %s ==\n", title);
}

static int tl_summary(void)
{
    printf("-- %d/%d tests passed%s --\n", tl_passed, tl_total,
           tl_passed == tl_total ? "  (all good!)" : "");
    return tl_passed == tl_total ? 0 : 1;
}

/* ---- small helpers used by several test files --------------------------- */

/* Build "<tmpdir>/<name>" in out. */
static inline void tl_path(char *out, size_t n, const char *name)
{
    snprintf(out, n, "%s/%s", tl_tmpdir, name);
}

static inline void tl_write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) { perror("tl_write_file"); _exit(2); }
    fputs(content, f);
    fclose(f);
}

/* Returns a malloc'ed copy of the file's content, or NULL if it can't be read.
 * *len (optional) receives the length in bytes. */
static inline char *tl_read_file(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    char *buf;
    long n;
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc((size_t)n + 1);
    if (buf == NULL) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { /* short read: keep going */ }
    buf[n] = '\0';
    fclose(f);
    if (len) *len = (size_t)n;
    return buf;
}

/* CHECK that the file at path has exactly the given content. */
#define CHECK_FILE(path, expected)                                              \
    do {                                                                        \
        char *c_ = tl_read_file((path), NULL);                                  \
        if (c_ == NULL) {                                                       \
            fprintf(stderr, "         %s:%d: could not read %s\n", __FILE__,    \
                    __LINE__, (path));                                          \
            TL_FAIL_EXIT();                                                     \
        }                                                                       \
        if (strcmp(c_, (expected)) != 0) {                                      \
            fprintf(stderr,                                                     \
                    "         %s:%d: contents of %s\n"                          \
                    "             expected \"%s\", got \"%s\"\n",               \
                    __FILE__, __LINE__, (path), (expected), c_);                \
            TL_FAIL_EXIT();                                                     \
        }                                                                       \
        free(c_);                                                               \
    } while (0)

/* A FILE* positioned at the start of a (possibly empty) string. */
static inline FILE *tl_file_from_string(const char *s)
{
    FILE *f = tmpfile();
    if (f == NULL) { perror("tmpfile"); _exit(2); }
    fputs(s, f);
    rewind(f);
    return f;
}

/* The lowest unused file descriptor number. Comparing it before and after a
 * call is a simple way to detect leaked descriptors. */
static inline int tl_lowest_free_fd(void)
{
    int fd = open("/dev/null", O_RDONLY);
    if (fd >= 0) close(fd);
    return fd;
}

static inline void tl_sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

#endif /* TESTLIB_H */
