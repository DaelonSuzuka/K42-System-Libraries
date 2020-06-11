#include "shell_command_processor.h"
#include "serial_port.h"
#include "shell.h"
#include <string.h>

/* ************************************************************************** */

// returns an initialized shell_args_t object
shell_args_t new_args(void) {
    shell_args_t args;
    memset(&args, 0, sizeof(shell_args_t));

    return args;
}

/* ************************************************************************** */
/*  Shell Command List

    The shell command list is assembled at compile time. Each shell command
    definition must be accompanied by a call to the REGISTER_SHELL_COMMAND()
    macro. Here's an example shell command declaration and registration call.

        void shell_example(int argc, char **argv) { <snip> }
        REGISTER_SHELL_COMMAND(shell_example, "example");

    This macro expands to something like the following:
        const shell_command_t __section("shell_cmds") example_cmd =
        {shell_example, "example"};

    This creates a shell_command_t object named example_cmd, initializes it with
    a function pointer shell_example() and the string that should be matched
    with this shell command. This shell_command_t is marked const, so it lives
    in ROM, and the compiler/linker is instructed to place this object into a
    PSECT labeled "shell_cmds".

    The end result is that we're using the linker to construct an array of
    shell_command_t's. The entire process is a bit roundabout, but without it
    we'd have a gross dependency inversion where this file would need to include
    files from the user code and then call their init functions to register each
    command into the array. That process was very error prone and generated a
    lot of extra boilerplate code and extra noise in commits. It also had the
    unfortunate side-effect of causing this shell code to break if the project
    didn't have the expected files in the expected place.

    !This linker-array-construction does impose one requirement on the project:
    !The compiler MUST be called with "-Pshell_cmds" added to its flags.

    If this flag isn't added to the compiler, the Linker will not give values to
    the symblols _Lshell_cmds or _Hshell_cmds, which are vital to
    accessing the command table from C code.
*/

// The linker defines these during compilation.
extern const char _Lshell_cmds[]; // start of shell_cmds
extern const char _Hshell_cmds[]; // end of shell_cmds

// Use the linker-defined symbols to calculate how many commands are registered
uint8_t calculate_number_of_commands(void) {
    uint16_t commandListLength = &_Hshell_cmds - &_Lshell_cmds;
    return commandListLength / sizeof(shell_command_t);
}

// Create and initialize a pointer we can use to retrieve the command list
const shell_command_t *commandList =
    (const shell_command_t *)(_Lshell_cmds);

// Print all registered shell commands
void print_command_list(void) {
    for (uint8_t i = 0; i < calculate_number_of_commands(); i++) {
        println(commandList[i].command);
    }
}

/* ************************************************************************** */

// returns the index in list whose command matches the given string
int8_t find_command_in_list(char *string) {
    for (uint8_t i = 0; i < calculate_number_of_commands(); i++) {
        // If string matches one on the list
        if (!strcmp(string, commandList[i].command)) {
            return i;
        }
    }
    return -1;
}

// parses a line's buffer into a shell_args_t object
shell_args_t parse_shell_line(shell_line_t *line) {
    shell_args_t args = new_args();

    char *token = strtok(&line->buffer[0], " ");
    while (token != NULL && args.argc < CONFIG_SHELL_MAX_COMMAND_ARGS) {
        args.argv[args.argc++] = token;
        token = strtok(NULL, " ");
    }

    return args;
}

int8_t process_shell_command(shell_line_t *line) {
    shell_args_t args = parse_shell_line(line);

    // figure out which command matches the received string
    int8_t command = find_command_in_list(args.argv[0]);

    // if we found a valid command, execute it
    if (command != -1) {
        commandList[command].program(args.argc, args.argv);

        return 0;
    }
    return -1;
}