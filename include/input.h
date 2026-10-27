/*
 * input.h - reading and cleaning up one line of input (Milestone 1)
 *
 * This header is the specification for input.c. Do not change it.
 */
#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdio.h>

#define READ_EOF      (-1)  /* end of input, nothing was read                    */
#define READ_TOO_LONG (-2)  /* the line did not fit in the buffer                */

/* Read one line from in into buf (capacity size bytes, size >= 2).
 *
 * On success: the trailing '\n' is NOT stored, buf is '\0'-terminated, and the
 * return value is the length of the line (0 for an empty line).
 *   - A line that is exactly size-1 characters long still fits.
 *   - A last line that has no trailing '\n' is returned normally; the NEXT call
 *     then returns READ_EOF.
 * Returns READ_EOF if the input is at end-of-file and nothing was read.
 * Returns READ_TOO_LONG if the line has more than size-1 characters. In that
 * case the REST OF THAT LINE IS DISCARDED (up to and including its '\n', or
 * to end-of-file), so that the next call starts cleanly on the next line.
 * The contents of buf after READ_TOO_LONG are unspecified.
 *
 * Hint: fgets() does most of the work; the interesting part is noticing that
 * the line was cut off and throwing away the remainder. */
int read_line(FILE *in, char *buf, size_t size);

/* Remove leading and trailing whitespace (as defined by isspace()) from the
 * string s, IN PLACE. Returns a pointer to the first non-whitespace character
 * of s (so the result may point into the middle of the original buffer).
 * Whitespace inside the string is untouched. An all-whitespace or empty string
 * gives "". Returns NULL if s is NULL. */
char *str_trim(char *s);

#endif /* INPUT_H */
