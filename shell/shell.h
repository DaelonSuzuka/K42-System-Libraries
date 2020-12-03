#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "peripherals/uart.h"
#include "shell_config.h"
#include "shell_command_utils.h"
#include <stdint.h>

/* -------------------------------------------------------------------------- */

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

// setup
extern void shell_init(uart_config_t *config);

/*	Main Shell processing

    This function implements the main functionality of the command line
    interface this function should be called frequently so it can handle the
    input from the data stream.
*/
extern void shell_update(void);

#endif // _SHELL_H_