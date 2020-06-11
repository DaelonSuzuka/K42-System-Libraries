#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include "shell.h"
#include "shell_config.h"
#include "shell_command_utils.h"
#include <stdint.h>

/* ************************************************************************** */

typedef struct {
    uint8_t argc;
    char *argv[CONFIG_SHELL_MAX_COMMAND_ARGS];
} shell_args_t;

extern shell_args_t new_args(void);

/* ************************************************************************** */

// prints all commands that are registered in the master command list
extern void print_command_list(void);

/* -------------------------------------------------------------------------- */

// parses a line's buffer into a shell_args_t object
extern shell_args_t parse_shell_line(shell_line_t *line);

// Attempt to parse the shell input and excute the matching command
extern int8_t process_shell_command(shell_line_t *line);

#endif // _SHELL_COMMAND_PROCESSOR_H_
