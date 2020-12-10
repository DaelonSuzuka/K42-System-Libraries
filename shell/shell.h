#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "shell_config.h"
#include <stdint.h>

/* ************************************************************************** */

/*  shell_line_t

    A shell line contains a char buffer that is SHELL_MAX_LENGTH long, and two
    variables to keep track of the current length of the buffer and the current
    location of the cursor.
*/
typedef struct {
    char buffer[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} shell_line_t;

// make sure there's a terminating null character at the end of the line
#define shell_add_terminator_to_line(line) line.buffer[line.length] = '\0'

// erase a line
#define shell_reset_line(line) memset(&line, 0, sizeof(shell_line_t));

/* -------------------------------------------------------------------------- */

extern shell_line_t shell;

/* ************************************************************************** */

//
extern void sh_print(const char *string);

//
extern void sh_println(const char *string);

/* -------------------------------------------------------------------------- */

// setup
extern void shell_init(void);

//
extern void shell_update(char currentChar);

#endif // _SHELL_H_