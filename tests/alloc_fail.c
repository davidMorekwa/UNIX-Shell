/*
 * alloc_fail.c - lets the tests make malloc/calloc/realloc fail on purpose.
 *
 * The test programs are linked with  -Wl,--wrap=malloc  (and calloc/realloc),
 * which redirects every call to malloc() in your code to __wrap_malloc() below.
 * This is how we check that your code handles "out of memory" correctly
 * without actually running out of memory. You do not need to edit this file.
 */
#include <stddef.h>
#include <stdlib.h>

void *__real_malloc(size_t n);
void *__real_calloc(size_t a, size_t b);
void *__real_realloc(void *p, size_t n);

static int g_countdown = -1; /* -1 = never fail */

static int should_fail(void)
{
    if (g_countdown < 0) return 0;
    if (g_countdown == 0) {
        g_countdown = -1; /* fail exactly once */
        return 1;
    }
    g_countdown--;
    return 0;
}

void tl_alloc_fail_after(int n) { g_countdown = n; }
void tl_alloc_reset(void) { g_countdown = -1; }

void *__wrap_malloc(size_t n)
{
    if (should_fail()) return NULL;
    return __real_malloc(n);
}

void *__wrap_calloc(size_t a, size_t b)
{
    if (should_fail()) return NULL;
    return __real_calloc(a, b);
}

void *__wrap_realloc(void *p, size_t n)
{
    if (should_fail()) return NULL;
    return __real_realloc(p, n);
}
