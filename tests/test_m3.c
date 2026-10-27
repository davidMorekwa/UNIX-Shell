/* test_m3.c - unit tests for Milestone 3 (Week 3): iohelp (read/write),
 * executor (redirection, fork/exec/wait, pipes, signals). Run with:  make test3
 *
 * Milestone 1 (command.c, shell_state.c) must already work: these tests build
 * their commands with command_init / command_add_arg / command_set_*.
 *
 * The tests run real programs (echo, cat, sort, sh, wc, head), so they need a
 * normal Linux userland. */
#include "testlib.h"

#include <dirent.h>
#include <sys/stat.h>

#include "command.h"
#include "executor.h"
#include "iohelp.h"
#include "shell_state.h"

#define END ((const char *)NULL)

/* ---------------------------------------------------------------- helpers */

static volatile sig_atomic_t got_usr1 = 0;
static void on_usr1(int sig) { (void)sig; got_usr1 = 1; }

/* Install a SIGUSR1 handler WITHOUT SA_RESTART, so that a blocked read() or
 * write() is really interrupted (fails with EINTR / returns a short count). */
static void install_usr1_handler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

static unsigned char pat(size_t i) { return (unsigned char)((i * 7 + i / 251) % 256); }

static unsigned char *make_pattern(size_t n)
{
    unsigned char *p = malloc(n);
    size_t i;
    CHECK(p != NULL);
    for (i = 0; i < n; i++) p[i] = pat(i);
    return p;
}

/* Run fn(arg) in a forked child and return the child's exit status (or
 * 128 + signal if it was killed). Used to test functions that change the
 * calling process (redirect_io). */
static int in_child(void (*fn)(void *), void *arg)
{
    int status;
    pid_t pid;
    fflush(NULL);
    pid = fork();
    CHECK(pid >= 0);
    if (pid == 0) {
        fn(arg);
        _exit(0);
    }
    CHECK(waitpid(pid, &status, 0) == pid);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return WEXITSTATUS(status);
}

/* Send our own stderr to a file; returns a saved copy to hand to stderr_restore. */
static int stderr_to_file(const char *path)
{
    int saved = dup(2);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(saved >= 0);
    CHECK(fd >= 0);
    CHECK(fcntl(saved, F_SETFD, FD_CLOEXEC) == 0); /* programs we exec must not see it */
    CHECK(dup2(fd, 2) >= 0);
    close(fd);
    return saved;
}

static void stderr_restore(int saved)
{
    dup2(saved, 2);
    close(saved);
}

/* Build a command from a NULL-terminated list of words. */
static void mkcmd(struct command *c, ...)
{
    va_list ap;
    const char *s;
    command_init(c);
    va_start(ap, c);
    while ((s = va_arg(ap, const char *)) != NULL) CHECK_INT(command_add_arg(c, s), 0);
    va_end(ap);
}

#define CMD(c, ...) mkcmd((c), __VA_ARGS__, END)

struct fdid { dev_t dev; ino_t ino; };

static void fd_identity(int fd, struct fdid *id)
{
    struct stat s;
    CHECK(fstat(fd, &s) == 0);
    id->dev = s.st_dev;
    id->ino = s.st_ino;
}

static int same_identity(const struct fdid *a, const struct fdid *b)
{
    return a->dev == b->dev && a->ino == b->ino;
}

/* How many descriptors this process has open (not counting the one used to look). */
static int count_open_fds(void)
{
    DIR *d = opendir("/proc/self/fd");
    struct dirent *e;
    int n = 0, self;
    CHECK(d != NULL);
    self = dirfd(d);
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.' || atoi(e->d_name) == self) continue;
        n++;
    }
    closedir(d);
    return n;
}

/* No child process should be left to wait for (no zombies, no strays). */
static void check_no_children(void)
{
    errno = 0;
    CHECK(waitpid(-1, NULL, WNOHANG) == -1);
    CHECK_INT(errno, ECHILD);
}

/* A child process that reads from fd and checks it receives exactly N pattern
 * bytes; exits 0 if so. */
static pid_t spawn_pattern_reader(int fd, int close_fd, size_t n)
{
    pid_t kid;
    fflush(NULL);
    kid = fork();
    CHECK(kid >= 0);
    if (kid == 0) {
        unsigned char buf[4096];
        size_t total = 0;
        ssize_t r, i;
        if (close_fd >= 0) close(close_fd);
        while ((r = read(fd, buf, sizeof buf)) > 0) {
            for (i = 0; i < r; i++)
                if (buf[i] != pat(total + (size_t)i)) _exit(2);
            total += (size_t)r;
        }
        _exit(total == n ? 0 : 3);
    }
    return kid;
}

static void expect_child_ok(pid_t kid)
{
    int status;
    CHECK(waitpid(kid, &status, 0) == kid);
    CHECK(WIFEXITED(status));
    CHECK_INT(WEXITSTATUS(status), 0);
}

/* ======================= iohelp.c ======================= */

static void test_write_all_writes_everything_to_a_file(void)
{
    enum { N = 100000 };
    char path[128];
    unsigned char *data = make_pattern(N);
    unsigned char *back;
    size_t len = 0;
    int fd;

    tl_path(path, sizeof path, "w.bin");
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(fd >= 0);
    CHECK_INT(write_all(fd, data, N), N);
    close(fd);

    back = (unsigned char *)tl_read_file(path, &len);
    CHECK(back != NULL);
    CHECK_INT(len, N);
    CHECK_INT(memcmp(back, data, N), 0);
    free(back);
    free(data);
}

static void test_write_all_zero_bytes_does_not_touch_the_descriptor(void)
{
    CHECK_INT(write_all(-1, "x", 0), 0);     /* n == 0: no write call, so no error */
}

static void test_write_all_reports_errors(void)
{
    errno = 0;
    CHECK_INT(write_all(-1, "x", 1), -1);
    CHECK_INT(errno, EBADF);
}

static void test_write_all_continues_after_an_interrupted_short_write(void)
{
    enum { N = 1 << 20 };
    unsigned char *data = make_pattern(N);
    int p[2];
    pid_t parent = getpid();
    pid_t kid;

    CHECK(pipe(p) == 0);
    install_usr1_handler();
    fflush(NULL);
    kid = fork();
    CHECK(kid >= 0);
    if (kid == 0) {
        /* The reader: let the parent fill the pipe and block in write(), then
         * interrupt it with a signal (a short write!), then drain everything. */
        unsigned char buf[4096];
        size_t total = 0;
        ssize_t r, i;
        close(p[1]);
        tl_sleep_ms(300);
        kill(parent, SIGUSR1);
        tl_sleep_ms(100);
        while ((r = read(p[0], buf, sizeof buf)) > 0) {
            for (i = 0; i < r; i++)
                if (buf[i] != pat(total + (size_t)i)) _exit(2);
            total += (size_t)r;
        }
        _exit(total == N ? 0 : 3);
    }
    close(p[0]);
    CHECK_INT(write_all(p[1], data, N), N);  /* must keep going after the signal */
    close(p[1]);
    expect_child_ok(kid);
    free(data);
}

static void test_write_all_retries_when_interrupted_before_writing_anything(void)
{
    int p[2];
    int flags;
    size_t filled = 0;
    char block[512];
    pid_t parent = getpid();
    pid_t kid;
    int done_blocks = 0;

    memset(block, 'a', sizeof block);
    CHECK(pipe(p) == 0);
    flags = fcntl(p[1], F_GETFL);
    CHECK(fcntl(p[1], F_SETFL, flags | O_NONBLOCK) == 0);
    for (;;) {                                 /* fill the pipe to the brim */
        ssize_t w = write(p[1], done_blocks ? "z" : block, done_blocks ? 1 : sizeof block);
        if (w < 0) {
            CHECK_INT(errno, EAGAIN);
            if (done_blocks) break;
            done_blocks = 1;
            continue;
        }
        filled += (size_t)w;
    }
    CHECK(fcntl(p[1], F_SETFL, flags) == 0);   /* blocking again, and the pipe is full */

    install_usr1_handler();
    fflush(NULL);
    kid = fork();
    CHECK(kid >= 0);
    if (kid == 0) {
        char buf[4096];
        size_t total = 0;
        ssize_t r;
        close(p[1]);
        tl_sleep_ms(300);                      /* the parent is now blocked in write()... */
        kill(parent, SIGUSR1);                 /* ...interrupt it: nothing was written: EINTR */
        tl_sleep_ms(100);
        while ((r = read(p[0], buf, sizeof buf)) > 0) total += (size_t)r;
        _exit(total == filled + 1 ? 0 : 3);
    }
    close(p[0]);
    CHECK_INT(write_all(p[1], "x", 1), 1);     /* EINTR with 0 bytes written: try again */
    close(p[1]);
    expect_child_ok(kid);
}

static void test_copy_fd_copies_a_large_file(void)
{
    enum { N = 1000000 };
    char in_path[128], out_path[128];
    unsigned char *data = make_pattern(N);
    unsigned char *back;
    size_t len = 0;
    int in_fd, out_fd;

    tl_path(in_path, sizeof in_path, "in.bin");
    tl_path(out_path, sizeof out_path, "out.bin");
    out_fd = open(in_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(out_fd >= 0);
    CHECK_INT(write(out_fd, data, N), N);
    close(out_fd);

    in_fd = open(in_path, O_RDONLY);
    out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(in_fd >= 0 && out_fd >= 0);
    CHECK_INT(copy_fd(in_fd, out_fd), N);
    close(in_fd);
    close(out_fd);

    back = (unsigned char *)tl_read_file(out_path, &len);
    CHECK(back != NULL);
    CHECK_INT(len, N);
    CHECK_INT(memcmp(back, data, N), 0);
    free(back);
    free(data);
}

static void test_copy_fd_empty_input(void)
{
    char in_path[128], out_path[128];
    int in_fd, out_fd;
    tl_path(in_path, sizeof in_path, "empty");
    tl_path(out_path, sizeof out_path, "out");
    tl_write_file(in_path, "");
    in_fd = open(in_path, O_RDONLY);
    out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(in_fd >= 0 && out_fd >= 0);
    CHECK_INT(copy_fd(in_fd, out_fd), 0);
    close(in_fd);
    close(out_fd);
    CHECK_FILE(out_path, "");
}

static void test_copy_fd_keeps_reading_until_end_of_file(void)
{
    char out_path[128];
    int p[2], out_fd;
    pid_t kid;

    tl_path(out_path, sizeof out_path, "out");
    CHECK(pipe(p) == 0);
    fflush(NULL);
    kid = fork();
    CHECK(kid >= 0);
    if (kid == 0) {                            /* a slow writer: two separate bursts */
        close(p[0]);
        if (write(p[1], "hello", 5) < 0) _exit(1);
        tl_sleep_ms(150);
        if (write(p[1], " world", 6) < 0) _exit(1);
        close(p[1]);
        _exit(0);
    }
    close(p[1]);
    out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(out_fd >= 0);
    CHECK_INT(copy_fd(p[0], out_fd), 11);      /* a short read is NOT the end */
    close(p[0]);
    close(out_fd);
    expect_child_ok(kid);
    CHECK_FILE(out_path, "hello world");
}

static void test_copy_fd_retries_an_interrupted_read(void)
{
    char out_path[128];
    int p[2], out_fd;
    pid_t parent = getpid();
    pid_t kid;

    tl_path(out_path, sizeof out_path, "out");
    install_usr1_handler();
    CHECK(pipe(p) == 0);
    fflush(NULL);
    kid = fork();
    CHECK(kid >= 0);
    if (kid == 0) {
        close(p[0]);
        tl_sleep_ms(100);
        kill(parent, SIGUSR1);                 /* interrupts the parent's blocked read */
        tl_sleep_ms(100);
        if (write(p[1], "after the signal", 16) < 0) _exit(1);
        close(p[1]);
        _exit(0);
    }
    close(p[1]);
    out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(out_fd >= 0);
    CHECK_INT(copy_fd(p[0], out_fd), 16);      /* EINTR is not an error */
    CHECK_INT(got_usr1, 1);                    /* (the signal really did arrive) */
    close(p[0]);
    close(out_fd);
    expect_child_ok(kid);
    CHECK_FILE(out_path, "after the signal");
}

static void test_copy_fd_into_a_pipe_bigger_than_its_buffer(void)
{
    enum { N = 300000 };
    char in_path[128];
    unsigned char *data = make_pattern(N);
    int p[2], in_fd, fd;
    pid_t kid;

    tl_path(in_path, sizeof in_path, "in.bin");
    fd = open(in_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK(fd >= 0);
    CHECK_INT(write(fd, data, N), N);
    close(fd);

    CHECK(pipe(p) == 0);
    kid = spawn_pattern_reader(p[0], p[1], N);
    close(p[0]);
    in_fd = open(in_path, O_RDONLY);
    CHECK(in_fd >= 0);
    CHECK_INT(copy_fd(in_fd, p[1]), N);
    close(in_fd);
    close(p[1]);
    expect_child_ok(kid);
    free(data);
}

static void test_copy_fd_reports_bad_descriptors(void)
{
    char path[128];
    int fd;
    tl_path(path, sizeof path, "data");
    tl_write_file(path, "data");

    errno = 0;
    CHECK_INT(copy_fd(-1, 1), -1);
    CHECK_INT(errno, EBADF);

    fd = open(path, O_RDONLY);
    CHECK(fd >= 0);
    errno = 0;
    CHECK_INT(copy_fd(fd, -1), -1);            /* there IS data to write, so it fails */
    CHECK_INT(errno, EBADF);
    close(fd);
}

static void test_copy_fd_does_not_close_the_descriptors(void)
{
    char in_path[128], out_path[128];
    int in_fd, out_fd;
    tl_path(in_path, sizeof in_path, "in");
    tl_path(out_path, sizeof out_path, "out");
    tl_write_file(in_path, "abc");
    in_fd = open(in_path, O_RDONLY);
    out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    CHECK_INT(copy_fd(in_fd, out_fd), 3);
    CHECK(fcntl(in_fd, F_GETFD) != -1);        /* still open: the caller closes them */
    CHECK(fcntl(out_fd, F_GETFD) != -1);
    close(in_fd);
    close(out_fd);
}

/* ======================= executor.c: redirect_io ======================= */

struct redir_args {
    struct command *cmd;
    const char *text;
};

static void child_write_to_stdout(void *p)
{
    struct redir_args *a = p;
    CHECK_INT(redirect_io(a->cmd), 0);
    CHECK(write(1, a->text, strlen(a->text)) == (ssize_t)strlen(a->text));
}

static void child_umask0_write_to_stdout(void *p)
{
    umask(0);                                  /* so the file mode is exactly what open() was given */
    child_write_to_stdout(p);
}

static void child_expect_stdin(void *p)
{
    struct redir_args *a = p;
    char buf[128];
    ssize_t n;
    CHECK_INT(redirect_io(a->cmd), 0);
    n = read(0, buf, sizeof buf - 1);
    CHECK(n >= 0);
    buf[n] = '\0';
    CHECK_STR(buf, a->text);
    CHECK_INT(read(0, buf, sizeof buf), 0);    /* and then end-of-file */
}

static void child_copy_stdin_to_stdout(void *p)
{
    struct redir_args *a = p;
    char buf[64];
    ssize_t n;
    CHECK_INT(redirect_io(a->cmd), 0);
    while ((n = read(0, buf, sizeof buf)) > 0) CHECK(write(1, buf, (size_t)n) == n);
}

static void child_closed_stdin(void *p)
{
    struct redir_args *a = p;
    char buf[64];
    ssize_t n;
    close(0);                                  /* now open() will hand out descriptor 0 */
    CHECK_INT(redirect_io(a->cmd), 0);
    n = read(0, buf, sizeof buf - 1);
    CHECK(n > 0);
    buf[n] = '\0';
    CHECK_STR(buf, a->text);
}

static void child_closed_stdout(void *p)
{
    struct redir_args *a = p;
    close(1);                                  /* now open() will hand out descriptor 1 */
    CHECK_INT(redirect_io(a->cmd), 0);
    CHECK(write(1, a->text, strlen(a->text)) == (ssize_t)strlen(a->text));
}

static void child_expect_untouched(void *p)
{
    struct redir_args *a = p;
    struct fdid in_before, out_before, in_after, out_after;
    int free_before;
    fd_identity(0, &in_before);
    fd_identity(1, &out_before);
    free_before = tl_lowest_free_fd();
    CHECK_INT(redirect_io(a->cmd), 0);
    fd_identity(0, &in_after);
    fd_identity(1, &out_after);
    CHECK(same_identity(&in_before, &in_after));
    CHECK(same_identity(&out_before, &out_after));
    CHECK_INT(tl_lowest_free_fd(), free_before);
}

static void child_expect_no_leak(void *p)
{
    struct redir_args *a = p;
    int free_before = tl_lowest_free_fd();
    CHECK_INT(redirect_io(a->cmd), 0);
    CHECK_INT(tl_lowest_free_fd(), free_before);   /* the temporary descriptors are closed */
}

struct fail_args {
    struct command *cmd;
    int expected_errno;
};

static void child_expect_clean_failure(void *p)
{
    struct fail_args *a = p;
    struct fdid in_before, out_before, in_after, out_after;
    int free_before, rc;

    fd_identity(0, &in_before);
    fd_identity(1, &out_before);
    free_before = tl_lowest_free_fd();
    errno = 0;
    rc = redirect_io(a->cmd);
    CHECK_INT(rc, -1);
    CHECK_INT(errno, a->expected_errno);
    fd_identity(0, &in_after);
    fd_identity(1, &out_after);
    CHECK(same_identity(&in_before, &in_after));   /* stdin untouched  */
    CHECK(same_identity(&out_before, &out_after)); /* stdout untouched */
    CHECK_INT(tl_lowest_free_fd(), free_before);   /* nothing leaked   */
}

static void test_redirect_output_truncates(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "out.txt");
    tl_write_file(path, "old content that is much longer");
    CMD(&c, "x");
    command_set_outfile(&c, path, 0);
    a.cmd = &c;
    a.text = "new";
    CHECK_INT(in_child(child_write_to_stdout, &a), 0);
    CHECK_FILE(path, "new");
}

static void test_redirect_output_appends(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "log.txt");
    tl_write_file(path, "old\n");
    CMD(&c, "x");
    command_set_outfile(&c, path, 1);
    a.cmd = &c;
    a.text = "new\n";
    CHECK_INT(in_child(child_write_to_stdout, &a), 0);
    CHECK_FILE(path, "old\nnew\n");
}

static void test_redirect_output_creates_the_file_with_mode_0644(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    struct stat st;
    tl_path(path, sizeof path, "created.txt");
    CMD(&c, "x");
    command_set_outfile(&c, path, 0);
    a.cmd = &c;
    a.text = "hi";
    CHECK_INT(in_child(child_umask0_write_to_stdout, &a), 0);
    CHECK_FILE(path, "hi");
    CHECK(stat(path, &st) == 0);
    CHECK_INT(st.st_mode & 0777, 0644);
}

static void test_redirect_append_creates_a_missing_file(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "fresh.log");
    CMD(&c, "x");
    command_set_outfile(&c, path, 1);
    a.cmd = &c;
    a.text = "first\n";
    CHECK_INT(in_child(child_write_to_stdout, &a), 0);
    CHECK_FILE(path, "first\n");
}

static void test_redirect_input(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "in.txt");
    tl_write_file(path, "from the file\n");
    CMD(&c, "x");
    command_set_infile(&c, path);
    a.cmd = &c;
    a.text = "from the file\n";
    CHECK_INT(in_child(child_expect_stdin, &a), 0);
}

static void test_redirect_input_and_output_together(void)
{
    char in_path[128], out_path[128];
    struct command c;
    struct redir_args a;
    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "out.txt");
    tl_write_file(in_path, "line one\nline two\n");
    CMD(&c, "x");
    command_set_infile(&c, in_path);
    command_set_outfile(&c, out_path, 0);
    a.cmd = &c;
    a.text = "";
    CHECK_INT(in_child(child_copy_stdin_to_stdout, &a), 0);
    CHECK_FILE(out_path, "line one\nline two\n");
}

static void test_redirect_with_nothing_to_do(void)
{
    struct command c;
    struct redir_args a;
    CMD(&c, "x");
    a.cmd = &c;
    a.text = "";
    CHECK_INT(in_child(child_expect_untouched, &a), 0);
}

static void test_redirect_leaves_no_extra_descriptors(void)
{
    char in_path[128], out_path[128];
    struct command c;
    struct redir_args a;
    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "out.txt");
    tl_write_file(in_path, "x");
    CMD(&c, "x");
    command_set_infile(&c, in_path);
    command_set_outfile(&c, out_path, 0);
    a.cmd = &c;
    a.text = "";
    CHECK_INT(in_child(child_expect_no_leak, &a), 0);
}

static void test_redirect_missing_input_file_fails_cleanly(void)
{
    char missing[128], out_path[128];
    struct command c;
    struct fail_args a;
    tl_path(missing, sizeof missing, "does_not_exist");
    tl_path(out_path, sizeof out_path, "out.txt");
    CMD(&c, "x");
    command_set_infile(&c, missing);
    command_set_outfile(&c, out_path, 0);
    a.cmd = &c;
    a.expected_errno = ENOENT;
    CHECK_INT(in_child(child_expect_clean_failure, &a), 0);
    CHECK(access(out_path, F_OK) != 0);        /* the output file must not even be created */
}

static void test_redirect_unwritable_output_fails_cleanly(void)
{
    char in_path[128], out_path[128];
    struct command c;
    struct fail_args a;
    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "no_such_dir/out.txt");
    tl_write_file(in_path, "data");
    CMD(&c, "x");
    command_set_infile(&c, in_path);           /* this open succeeds and must be closed again */
    command_set_outfile(&c, out_path, 0);
    a.cmd = &c;
    a.expected_errno = ENOENT;
    CHECK_INT(in_child(child_expect_clean_failure, &a), 0);
}

static void test_redirect_when_stdin_is_closed(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "in.txt");
    tl_write_file(path, "still works");
    CMD(&c, "x");
    command_set_infile(&c, path);
    a.cmd = &c;
    a.text = "still works";
    /* open() returns 0 here; dup2(0, 0) does nothing, and close(0) afterwards
     * would throw the file away. */
    CHECK_INT(in_child(child_closed_stdin, &a), 0);
}

static void test_redirect_when_stdout_is_closed(void)
{
    char path[128];
    struct command c;
    struct redir_args a;
    tl_path(path, sizeof path, "out.txt");
    CMD(&c, "x");
    command_set_outfile(&c, path, 0);
    a.cmd = &c;
    a.text = "still works";
    CHECK_INT(in_child(child_closed_stdout, &a), 0);
    CHECK_FILE(path, "still works");
}

/* ======================= executor.c: execute_command ======================= */

static void test_execute_runs_a_program_with_output_redirected(void)
{
    char path[128];
    struct command c;
    struct shell_state st;
    tl_path(path, sizeof path, "out.txt");
    shell_state_init(&st);
    CMD(&c, "echo", "hello");
    command_set_outfile(&c, path, 0);
    CHECK_INT(execute_command(&c, &st), 0);
    CHECK_FILE(path, "hello\n");
    CHECK_INT(st.last_status, 0);
}

static void test_execute_passes_the_arguments_through(void)
{
    char path[128];
    struct command c;
    struct shell_state st;
    tl_path(path, sizeof path, "out.txt");
    shell_state_init(&st);
    /* sh -c 'echo "$0|$1"' first "second second"  ->  first|second second */
    CMD(&c, "sh", "-c", "echo \"$0|$1\"", "first", "second second");
    command_set_outfile(&c, path, 0);
    CHECK_INT(execute_command(&c, &st), 0);
    CHECK_FILE(path, "first|second second\n");
}

static void test_execute_returns_the_exit_status(void)
{
    struct command c;
    struct shell_state st;
    shell_state_init(&st);
    CMD(&c, "sh", "-c", "exit 7");
    CHECK_INT(execute_command(&c, &st), 7);
    CHECK_INT(st.last_status, 7);
    CMD(&c, "sh", "-c", "exit 255");
    CHECK_INT(execute_command(&c, &st), 255);
    CMD(&c, "sh", "-c", "exit 0");
    CHECK_INT(execute_command(&c, &st), 0);
    CHECK_INT(st.last_status, 0);
}

static void test_execute_reports_death_by_signal_as_128_plus_signal(void)
{
    struct command c;
    struct shell_state st;
    shell_state_init(&st);
    CMD(&c, "sh", "-c", "kill -TERM $$");
    CHECK_INT(execute_command(&c, &st), 128 + SIGTERM);
    CHECK_INT(st.last_status, 143);
}

static void test_execute_program_not_found_is_127(void)
{
    char err[128];
    char *msg;
    struct command c;
    struct shell_state st;
    int saved, rc;
    tl_path(err, sizeof err, "stderr.txt");
    shell_state_init(&st);
    CMD(&c, "no_such_program_xyz");
    saved = stderr_to_file(err);
    rc = execute_command(&c, &st);
    stderr_restore(saved);
    CHECK_INT(rc, 127);
    CHECK_INT(st.last_status, 127);
    msg = tl_read_file(err, NULL);
    CHECK(msg != NULL);
    CHECK(strstr(msg, "mysh:") != NULL);                 /* the message names the shell... */
    CHECK(strstr(msg, "no_such_program_xyz") != NULL);   /* ...and the program */
    free(msg);
}

static void test_execute_not_executable_is_126(void)
{
    char script[128], err[128];
    struct command c;
    struct shell_state st;
    int saved, rc;
    tl_path(script, sizeof script, "script.sh");
    tl_path(err, sizeof err, "stderr.txt");
    tl_write_file(script, "#!/bin/sh\necho hi\n");
    CHECK(chmod(script, 0644) == 0);                     /* no execute permission */
    shell_state_init(&st);
    CMD(&c, script);
    saved = stderr_to_file(err);
    rc = execute_command(&c, &st);
    stderr_restore(saved);
    CHECK_INT(rc, 126);
    CHECK_INT(st.last_status, 126);
}

static void test_execute_input_redirect(void)
{
    char in_path[128], out_path[128];
    struct command c;
    struct shell_state st;
    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "out.txt");
    tl_write_file(in_path, "line one\nline two\n");
    shell_state_init(&st);
    CMD(&c, "cat");
    command_set_infile(&c, in_path);
    command_set_outfile(&c, out_path, 0);
    CHECK_INT(execute_command(&c, &st), 0);
    CHECK_FILE(out_path, "line one\nline two\n");
}

static void test_execute_append_redirect(void)
{
    char path[128];
    struct command c;
    struct shell_state st;
    tl_path(path, sizeof path, "log.txt");
    tl_write_file(path, "one\n");
    shell_state_init(&st);
    CMD(&c, "echo", "two");
    command_set_outfile(&c, path, 1);
    CHECK_INT(execute_command(&c, &st), 0);
    CHECK_FILE(path, "one\ntwo\n");
}

static void test_execute_failed_redirect_is_status_1_and_the_program_never_runs(void)
{
    char missing[128], marker[128], err[128], script[256];
    struct command c;
    struct shell_state st;
    int saved, rc;
    tl_path(missing, sizeof missing, "no_such_input");
    tl_path(marker, sizeof marker, "marker");
    tl_path(err, sizeof err, "stderr.txt");
    snprintf(script, sizeof script, "echo ran > '%s'", marker);
    shell_state_init(&st);
    CMD(&c, "sh", "-c", script);
    command_set_infile(&c, missing);
    saved = stderr_to_file(err);
    rc = execute_command(&c, &st);
    stderr_restore(saved);
    CHECK_INT(rc, 1);
    CHECK_INT(st.last_status, 1);
    CHECK(access(marker, F_OK) != 0);                    /* the command did not run */
}

static void test_execute_does_not_disturb_the_parent(void)
{
    char in_path[128], out_path[128];
    struct command c;
    struct shell_state st;
    struct fdid in_before, out_before, in_after, out_after;
    int free_before;

    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "out.txt");
    tl_write_file(in_path, "x\n");
    shell_state_init(&st);
    CMD(&c, "cat");
    command_set_infile(&c, in_path);
    command_set_outfile(&c, out_path, 0);

    fd_identity(0, &in_before);
    fd_identity(1, &out_before);
    free_before = tl_lowest_free_fd();
    CHECK_INT(execute_command(&c, &st), 0);
    fd_identity(0, &in_after);
    fd_identity(1, &out_after);
    CHECK(same_identity(&in_before, &in_after));         /* the SHELL's stdin/stdout are unchanged */
    CHECK(same_identity(&out_before, &out_after));
    CHECK_INT(tl_lowest_free_fd(), free_before);
}

static void test_execute_leaves_no_zombies(void)
{
    struct command c;
    struct shell_state st;
    char err[128];
    int saved;
    tl_path(err, sizeof err, "stderr.txt");
    shell_state_init(&st);

    CMD(&c, "sh", "-c", "exit 0");
    CHECK_INT(execute_command(&c, &st), 0);
    check_no_children();

    CMD(&c, "no_such_program_xyz");
    saved = stderr_to_file(err);
    CHECK_INT(execute_command(&c, &st), 127);
    stderr_restore(saved);
    check_no_children();
}

static void test_execute_many_times_without_leaking(void)
{
    char out_path[128];
    struct command c;
    struct shell_state st;
    int free_before, i;
    tl_path(out_path, sizeof out_path, "out.txt");
    shell_state_init(&st);
    free_before = tl_lowest_free_fd();
    for (i = 0; i < 30; i++) {
        CMD(&c, "sh", "-c", "exit 0");
        command_set_outfile(&c, out_path, 0);
        CHECK_INT(execute_command(&c, &st), 0);
    }
    CHECK_INT(tl_lowest_free_fd(), free_before);
    check_no_children();
}

static void test_execute_rejects_an_empty_command(void)
{
    struct command c;
    struct shell_state st;
    command_init(&c);
    shell_state_init(&st);
    shell_state_set_status(&st, 5);
    CHECK_INT(execute_command(&c, &st), -1);
    CHECK_INT(st.last_status, 5);                        /* the state is untouched */
    CHECK_INT(execute_command(NULL, &st), -1);
    check_no_children();                                 /* and nothing was forked */
}

static void test_execute_accepts_a_null_state(void)
{
    struct command c;
    CMD(&c, "sh", "-c", "exit 4");
    CHECK_INT(execute_command(&c, NULL), 4);
}

/* valgrind makes a child that calls _exit() flush stdio anyway, which would make
 * the next test fail for a correct program. Detect it and skip. */
static int running_under_valgrind(void)
{
    const char *p = getenv("LD_PRELOAD");
    return p != NULL && strstr(p, "vgpreload") != NULL;
}

static void test_execute_failing_child_must_not_flush_the_parents_buffers(void)
{
    char data1[128], data2[128], missing[128], err[128];
    struct command c;
    struct shell_state st;
    FILE *f1, *f2;
    int saved;

    if (running_under_valgrind()) {
        fprintf(stderr, "         (skipped under valgrind: it flushes stdio in "
                        "children that call _exit; run this test without it)\n");
        return;
    }

    tl_path(data1, sizeof data1, "buffered1.txt");
    tl_path(data2, sizeof data2, "buffered2.txt");
    tl_path(missing, sizeof missing, "no_such_input");
    tl_path(err, sizeof err, "stderr.txt");
    shell_state_init(&st);

    /* Data sitting in the parent's stdio buffer, not yet written. If a failing
     * child calls exit() instead of _exit(), it flushes its COPY of that
     * buffer and the data ends up in the file twice. */
    f1 = fopen(data1, "w");
    CHECK(f1 != NULL);
    fputs("X", f1);
    CMD(&c, "no_such_program_xyz");
    saved = stderr_to_file(err);
    CHECK_INT(execute_command(&c, &st), 127);
    stderr_restore(saved);
    fclose(f1);
    CHECK_FILE(data1, "X");

    f2 = fopen(data2, "w");
    CHECK(f2 != NULL);
    fputs("Y", f2);
    CMD(&c, "echo", "hi");
    command_set_infile(&c, missing);                     /* the redirection fails this time */
    saved = stderr_to_file(err);
    CHECK_INT(execute_command(&c, &st), 1);
    stderr_restore(saved);
    fclose(f2);
    CHECK_FILE(data2, "Y");
}

static void test_execute_child_gets_default_sigint_even_if_the_shell_ignores_it(void)
{
    struct command c;
    struct shell_state st;
    shell_state_init(&st);
    shell_install_signal_handlers();                     /* the shell ignores Ctrl-C... */
    CMD(&c, "sh", "-c", "kill -INT $$");
    /* ...but "ignored" is inherited across exec, so the child must put the
     * default action back; otherwise it survives and reports 0. */
    CHECK_INT(execute_command(&c, &st), 128 + SIGINT);
}

/* ======================= executor.c: execute_pipe2 ======================= */

static void test_pipe_connects_two_commands(void)
{
    char path[128];
    struct command l, r;
    struct shell_state st;
    tl_path(path, sizeof path, "out.txt");
    shell_state_init(&st);
    CMD(&l, "echo", "hello");
    CMD(&r, "cat");
    command_set_outfile(&r, path, 0);
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    CHECK_FILE(path, "hello\n");
}

static void test_pipe_data_flows_through_a_filter(void)
{
    char path[128];
    struct command l, r;
    struct shell_state st;
    tl_path(path, sizeof path, "out.txt");
    shell_state_init(&st);
    CMD(&l, "sh", "-c", "printf 'b\\na\\nc\\n'");
    CMD(&r, "sort");
    command_set_outfile(&r, path, 0);
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    CHECK_FILE(path, "a\nb\nc\n");
}

static void test_pipe_each_side_keeps_its_own_redirection(void)
{
    char in_path[128], out_path[128];
    struct command l, r;
    struct shell_state st;
    tl_path(in_path, sizeof in_path, "in.txt");
    tl_path(out_path, sizeof out_path, "out.txt");
    tl_write_file(in_path, "1\n3\n2\n");
    shell_state_init(&st);
    CMD(&l, "cat");
    command_set_infile(&l, in_path);                     /* cat < in | sort -r > out */
    CMD(&r, "sort", "-r");
    command_set_outfile(&r, out_path, 0);
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    CHECK_FILE(out_path, "3\n2\n1\n");
}

static void test_pipe_status_is_that_of_the_right_hand_command(void)
{
    struct command l, r;
    struct shell_state st;
    shell_state_init(&st);

    CMD(&l, "echo", "x");
    CMD(&r, "sh", "-c", "exit 3");
    CHECK_INT(execute_pipe2(&l, &r, &st), 3);
    CHECK_INT(st.last_status, 3);

    CMD(&l, "sh", "-c", "exit 5");
    CMD(&r, "cat");
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);            /* the left status is ignored */
    CHECK_INT(st.last_status, 0);
}

static void test_pipe_with_a_missing_program(void)
{
    char err[128];
    struct command l, r;
    struct shell_state st;
    int saved, rc;
    tl_path(err, sizeof err, "stderr.txt");
    shell_state_init(&st);

    CMD(&l, "no_such_program_xyz");
    CMD(&r, "cat");
    saved = stderr_to_file(err);
    rc = execute_pipe2(&l, &r, &st);
    stderr_restore(saved);
    CHECK_INT(rc, 0);                                    /* cat just sees an empty input */

    CMD(&l, "echo", "x");
    CMD(&r, "no_such_program_xyz");
    saved = stderr_to_file(err);
    rc = execute_pipe2(&l, &r, &st);
    stderr_restore(saved);
    CHECK_INT(rc, 127);
    check_no_children();
}

static void test_pipe_moves_more_data_than_a_pipe_can_hold(void)
{
    char path[128];
    char *text;
    struct command l, r;
    struct shell_state st;
    tl_path(path, sizeof path, "count.txt");
    shell_state_init(&st);
    /* 300000 bytes is far more than the pipe buffer (64 KiB): this only works
     * if BOTH commands run at the same time. A shell that waits for the left
     * command to finish before starting the right one deadlocks (TIMEOUT). */
    CMD(&l, "head", "-c", "300000", "/dev/zero");
    CMD(&r, "wc", "-c");
    command_set_outfile(&r, path, 0);
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    text = tl_read_file(path, NULL);
    CHECK(text != NULL);
    CHECK_INT(atoi(text), 300000);
    free(text);
}

static void test_pipe_leaves_no_descriptors_or_zombies(void)
{
    struct command l, r;
    struct shell_state st;
    char path[128];
    int free_before, i;
    tl_path(path, sizeof path, "out.txt");
    shell_state_init(&st);
    free_before = tl_lowest_free_fd();
    for (i = 0; i < 10; i++) {
        CMD(&l, "echo", "x");
        CMD(&r, "cat");
        command_set_outfile(&r, path, 0);
        CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    }
    CHECK_INT(tl_lowest_free_fd(), free_before);         /* the pipe ends were all closed */
    check_no_children();
}

static void test_pipe_programs_do_not_inherit_the_pipe_ends(void)
{
    char err[128];
    struct command l, r;
    struct shell_state st;
    char *listing;
    int saved, expected, lines = 0;
    const char *c;

    if (running_under_valgrind()) {
        fprintf(stderr, "         (skipped under valgrind: it keeps extra descriptors "
                        "open in every process)\n");
        return;
    }
    tl_path(err, sizeof err, "fds.txt");
    shell_state_init(&st);
    expected = count_open_fds();               /* 3 normally: stdin, stdout, stderr */

    /* Each side runs a shell that lists ITS OWN open descriptors on stderr, one
     * per line. A leftover copy of a pipe end would make a list longer. */
    CMD(&l, "sh", "-c", "exec 1>&2; ls /proc/$$/fd");
    CMD(&r, "sh", "-c", "exec 1>&2; ls /proc/$$/fd; cat > /dev/null");
    saved = stderr_to_file(err);
    CHECK_INT(execute_pipe2(&l, &r, &st), 0);
    stderr_restore(saved);

    listing = tl_read_file(err, NULL);
    CHECK(listing != NULL);
    for (c = listing; *c != '\0'; c++)
        if (*c == '\n') lines++;
    CHECK_INT(lines, 2 * expected);            /* both programs see only 0, 1, 2 */
    free(listing);
}

static void test_pipe_children_get_default_sigint(void)
{
    struct command l, r;
    struct shell_state st;
    shell_state_init(&st);
    shell_install_signal_handlers();
    CMD(&l, "echo", "x");
    CMD(&r, "sh", "-c", "kill -INT $$");
    CHECK_INT(execute_pipe2(&l, &r, &st), 128 + SIGINT);
}

static void test_pipe_rejects_empty_commands_without_leaking(void)
{
    struct command l, r;
    struct shell_state st;
    int free_before;
    shell_state_init(&st);
    command_init(&l);
    CMD(&r, "cat");
    free_before = tl_lowest_free_fd();
    CHECK_INT(execute_pipe2(&l, &r, &st), -1);
    CHECK_INT(execute_pipe2(&r, &l, &st), -1);
    CHECK_INT(execute_pipe2(NULL, &r, &st), -1);
    CHECK_INT(tl_lowest_free_fd(), free_before);
    check_no_children();
}

/* ======================= executor.c: signals ======================= */

static void test_shell_ignores_interrupt_quit_and_stop(void)
{
    struct sigaction sa;
    shell_install_signal_handlers();
    CHECK(sigaction(SIGINT, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_IGN);
    CHECK(sigaction(SIGQUIT, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_IGN);
    CHECK(sigaction(SIGTSTP, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_IGN);
    CHECK(sigaction(SIGTERM, NULL, &sa) == 0);           /* other signals are left alone */
    CHECK(sa.sa_handler == SIG_DFL);
    raise(SIGINT);                                       /* would kill us if not ignored */
}

static void test_child_signals_are_reset_to_default(void)
{
    struct sigaction sa;
    shell_install_signal_handlers();
    shell_reset_child_signals();
    CHECK(sigaction(SIGINT, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_DFL);
    CHECK(sigaction(SIGQUIT, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_DFL);
    CHECK(sigaction(SIGTSTP, NULL, &sa) == 0);
    CHECK(sa.sa_handler == SIG_DFL);
}

int main(void)
{
    tl_banner("Milestone 3 (Week 3): file descriptors, fork/exec, pipes, signals");

    printf("iohelp.c\n");
    RUN(test_write_all_writes_everything_to_a_file);
    RUN(test_write_all_zero_bytes_does_not_touch_the_descriptor);
    RUN(test_write_all_reports_errors);
    RUN(test_write_all_continues_after_an_interrupted_short_write);
    RUN(test_write_all_retries_when_interrupted_before_writing_anything);
    RUN(test_copy_fd_copies_a_large_file);
    RUN(test_copy_fd_empty_input);
    RUN(test_copy_fd_keeps_reading_until_end_of_file);
    RUN(test_copy_fd_retries_an_interrupted_read);
    RUN(test_copy_fd_into_a_pipe_bigger_than_its_buffer);
    RUN(test_copy_fd_reports_bad_descriptors);
    RUN(test_copy_fd_does_not_close_the_descriptors);

    printf("executor.c: redirect_io\n");
    RUN(test_redirect_output_truncates);
    RUN(test_redirect_output_appends);
    RUN(test_redirect_output_creates_the_file_with_mode_0644);
    RUN(test_redirect_append_creates_a_missing_file);
    RUN(test_redirect_input);
    RUN(test_redirect_input_and_output_together);
    RUN(test_redirect_with_nothing_to_do);
    RUN(test_redirect_leaves_no_extra_descriptors);
    RUN(test_redirect_missing_input_file_fails_cleanly);
    RUN(test_redirect_unwritable_output_fails_cleanly);
    RUN(test_redirect_when_stdin_is_closed);
    RUN(test_redirect_when_stdout_is_closed);

    printf("executor.c: execute_command\n");
    RUN(test_execute_runs_a_program_with_output_redirected);
    RUN(test_execute_passes_the_arguments_through);
    RUN(test_execute_returns_the_exit_status);
    RUN(test_execute_reports_death_by_signal_as_128_plus_signal);
    RUN(test_execute_program_not_found_is_127);
    RUN(test_execute_not_executable_is_126);
    RUN(test_execute_input_redirect);
    RUN(test_execute_append_redirect);
    RUN(test_execute_failed_redirect_is_status_1_and_the_program_never_runs);
    RUN(test_execute_does_not_disturb_the_parent);
    RUN(test_execute_leaves_no_zombies);
    RUN(test_execute_many_times_without_leaking);
    RUN(test_execute_rejects_an_empty_command);
    RUN(test_execute_accepts_a_null_state);
    RUN(test_execute_failing_child_must_not_flush_the_parents_buffers);
    RUN(test_execute_child_gets_default_sigint_even_if_the_shell_ignores_it);

    printf("executor.c: execute_pipe2\n");
    RUN(test_pipe_connects_two_commands);
    RUN(test_pipe_data_flows_through_a_filter);
    RUN(test_pipe_each_side_keeps_its_own_redirection);
    RUN(test_pipe_status_is_that_of_the_right_hand_command);
    RUN(test_pipe_with_a_missing_program);
    RUN(test_pipe_moves_more_data_than_a_pipe_can_hold);
    RUN(test_pipe_leaves_no_descriptors_or_zombies);
    RUN(test_pipe_programs_do_not_inherit_the_pipe_ends);
    RUN(test_pipe_children_get_default_sigint);
    RUN(test_pipe_rejects_empty_commands_without_leaking);

    printf("executor.c: signals\n");
    RUN(test_shell_ignores_interrupt_quit_and_stop);
    RUN(test_child_signals_are_reset_to_default);

    return tl_summary();
}
