#!/bin/sh
# lint.sh - checks the milestone-specific "you must write this yourself" rules.
# usage: sh tests/lint.sh [srcdir]      (default: src)
#
# These rules exist so that the exercise teaches what it is meant to teach.
# Note: the check is a plain text search, so avoid writing the forbidden
# function names followed by "(" even inside your comments.

dir=${1:-src}
fail=0

check() { # file  regex  message
    f="$dir/$1"
    [ -f "$f" ] || return 0
    if grep -nE "$2" "$f" > /dev/null 2>&1; then
        echo "LINT FAIL: $1: $3"
        grep -nE "$2" "$f" | head -3 | sed 's/^/    /'
        fail=1
    fi
}

check mystring.c '#include[[:space:]]*<string\.h>' \
    'mystring.c must not include <string.h>: write these functions yourself'
check mystring.c '(^|[^A-Za-z0-9_])(strlen|strcmp|strncmp|strcpy|strncpy|strchr|strrchr|strdup|strcat|strncat|strstr|memcpy|memmove|memset|memcmp)[[:space:]]*\(' \
    'mystring.c must not call libc string/memory functions'

check lexer.c '(^|[^A-Za-z0-9_])(strtok|strtok_r|strsep|sscanf)[[:space:]]*\(' \
    'lexer.c must do its own splitting (no strtok/strsep/sscanf)'

check iohelp.c '#include[[:space:]]*<stdio\.h>' \
    'iohelp.c must use read/write directly: do not include <stdio.h>'
check iohelp.c '(^|[^A-Za-z0-9_])(fopen|fclose|fread|fwrite|fgets|fgetc|getc|putc|fputs|fputc|fprintf|printf|puts|fscanf|fflush)[[:space:]]*\(' \
    'iohelp.c must use read/write directly (no stdio)'

check executor.c '(^|[^A-Za-z0-9_])(system|popen)[[:space:]]*\(' \
    'executor.c must use fork/exec directly (no system/popen)'

if [ "$fail" -eq 0 ]; then
    echo "lint: OK ($dir)"
fi
exit $fail
