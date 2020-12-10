#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include "shell.h"
#include "shell_config.h"
#include <stdint.h>

/* ************************************************************************** */

// Pointer to a shell command function
typedef void (*command_function_t)(int argc, char **argv);

// Structure for registering a shell command
typedef struct {
    command_function_t function; // Pointer to the command function
    const char *command;         // The command that needs to be typed
} shell_command_t;

// register a command and associate it with a name
extern void shell_register_command(command_function_t function, const char *command);

/* ************************************************************************** */

// Pointer to a shell callback
typedef int8_t (*shell_callback_t)(char currentChar);

extern void shell_register_callback(shell_callback_t callback);

/* ************************************************************************** */

// prints all commands that are registered in the master command list
extern void print_command_list(void);

/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t argc;
    char *argv[CONFIG_SHELL_MAX_COMMAND_ARGS];
} shell_args_t;

//
extern shell_args_t new_args(void);

// parses a line's buffer into a shell_args_t object
extern shell_args_t parse_shell_line(shell_line_t *line);

// Attempt to parse the shell input and excute the matching command
extern int8_t process_shell_command(shell_line_t *line);

#endif // _SHELL_COMMAND_PROCESSOR_H_
