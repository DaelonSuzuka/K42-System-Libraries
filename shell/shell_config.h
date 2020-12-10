#ifndef _SHELL_CONFIG_H_
#define _SHELL_CONFIG_H_

/* ************************************************************************** */
// Shell version
#define SHELL_V_MAJOR "0"
#define SHELL_V_MINOR "6"
#define SHELL_V_PATCH "0"

#define SHELL_VERSION SHELL_V_MAJOR "." SHELL_V_MINOR "." SHELL_V_PATCH

/* ************************************************************************** */
// Shell prompt options
#define SHELL_COLOR_ENABLED

#define SHELL_PROMPT_COLOR "\033[32m"
#define SHELL_PROMPT_RESET "\033[37m"
#define SHELL_PROMPT_CHARACTER "$"
#define SHELL_PROMPT_LENGTH 2

#ifdef SHELL_COLOR_ENABLED
#define SHELL_PROMPT_STRING SHELL_PROMPT_COLOR SHELL_PROMPT_CHARACTER SHELL_PROMPT_RESET " "
#else
#define SHELL_PROMPT_STRING SHELL_PROMPT_CHARACTER " "
#endif

/* -------------------------------------------------------------------------- */
// Core shell options

// Defines the maximum number of commands that can be registered
#define MAXIMUM_NUM_OF_SHELL_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define SHELL_MAX_LENGTH 80

/* -------------------------------------------------------------------------- */
// Escape Sequence Processing options

// Configures the length of the buffer used to process escape sequences
#define SEQUENCE_BUFFER_LENGTH 10

/* -------------------------------------------------------------------------- */
// Command processing options

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

/* ************************************************************************** */
// Optional Features

/* -------------------------------------------------------------------------- */
// Shell history options

// Defines the number of lines stored in the shell history buffer
#define SHELL_HISTORY_LENGTH 4

#endif