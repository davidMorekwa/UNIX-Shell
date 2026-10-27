#!/bin/sh
# integration_m1.sh - runs the Milestone 1 shell (the main loop in main.c)
# with scripted input and compares what it prints.
# usage: sh tests/integration_m1.sh ./build/mysh_m1
#
# Only valid for Milestone 1: from Milestone 4 on, main.c will run commands
# instead of echoing them, and this script will no longer apply.
#
# Expected output is written with \n escapes so that the trailing space of the
# prompt ("mysh> ") is visible and cannot be lost by an editor.

BIN=${1:-./build/mysh_m1}
fail=0
total=0
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

show() { # print a file with newlines shown as \n
    sed 's/$/\\n/' "$1" | tr -d '\n'
    echo
}

# expect NAME EXPECTED_STDOUT EXPECTED_STDERR   (input is read from $tmp/in)
expect() {
    name=$1
    total=$((total + 1))
    "$BIN" < "$tmp/in" > "$tmp/out" 2> "$tmp/err"
    printf '%b' "$2" > "$tmp/want_out"
    printf '%b' "$3" > "$tmp/want_err"
    if cmp -s "$tmp/out" "$tmp/want_out" && cmp -s "$tmp/err" "$tmp/want_err"; then
        echo "  [PASS] $name"
    else
        fail=$((fail + 1))
        echo "  [FAIL] $name"
        echo "         expected stdout: $(show "$tmp/want_out")"
        echo "         actual   stdout: $(show "$tmp/out")"
        if ! cmp -s "$tmp/err" "$tmp/want_err"; then
            echo "         expected stderr: $(show "$tmp/want_err")"
            echo "         actual   stderr: $(show "$tmp/err")"
        fi
    fi
}

echo "== Milestone 1: main loop (integration) =="

printf 'hello world\nexit\n' > "$tmp/in"
expect "echoes a line, then exits" \
    'mysh> You said: hello world\nmysh> ' ''

printf 'hello world\n\n   spaced   \nexit\n' > "$tmp/in"
expect "blank lines are skipped, whitespace is trimmed" \
    'mysh> You said: hello world\nmysh> mysh> You said: spaced\nmysh> ' ''

printf 'abc\n' > "$tmp/in"
expect "Ctrl-D (end of input) prints a newline and quits" \
    'mysh> You said: abc\nmysh> \n' ''

printf 'exit\nshould never be read\n' > "$tmp/in"
expect "stops reading after exit" 'mysh> ' ''

printf '  exit  \n' > "$tmp/in"
expect "exit may have surrounding spaces" 'mysh> ' ''

printf 'no newline at the end' > "$tmp/in"
expect "last line without a newline still works" \
    'mysh> You said: no newline at the end\nmysh> \n' ''

# a 2000-character line, then a normal line
{ head -c 2000 /dev/zero | tr '\0' 'a'; printf '\nok\nexit\n'; } > "$tmp/in"
expect "too-long line: error on stderr, prompt shows [1], recovers" \
    'mysh> [1] mysh> You said: ok\nmysh> ' 'mysh: line too long\n'

if [ "$fail" -eq 0 ]; then
    echo "-- $total/$total integration tests passed  (all good!) --"
    exit 0
fi
echo "-- $((total - fail))/$total integration tests passed --"
exit 1
