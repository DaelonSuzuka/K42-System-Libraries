#ifndef _SHELL_COMMAND_UTILS_H_
#define _SHELL_COMMAND_UTILS_H_

#include "os/serial_port.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */
//! IMPORTANT:
/*  Registering a shell command

    Let's assume we want a shell command called 'hello' that just responds with
    'world!'. We can define that as:

    void sh_hello(int argc, char **argv) {
        print("world!");
    }

    Now we've got a shell command function, but the command processor doesn't
    know about it until we register it, aka associating the function with the
    string that the user types into the shell. Let's do that now:

    REGISTER_SHELL_COMMAND(sh_hello, "hello");

    This macro is a shortcut for creating a new shell_command_t struct,
    initializing it with a pointer to our command function(sh_hello), and the
    string literal of the desired command string. The struct is placed in ROM in
    a specially defined region of memory("shell_cmds").

    Assuming that the linker packs these structs together, the end result is an
    array of shell_command_t structs that the command processor can access at
    runtime.
*/

/* -------------------------------------------------------------------------- */

// Pointer to a shell program function
typedef void (*shell_program_t)(int argc, char **argv);

// Structure for registering a shell command
typedef struct {
    shell_program_t program; // A pointer to the shell command body.
    const char *command;     // The command that needs to be types
} shell_command_t;

// Macro to create a shell_command_t struct for you
#define REGISTER_SHELL_COMMAND(pointer, string)                                \
    const shell_command_t __section("shell_cmds")                          \
        pointer##_cmd = {pointer, string}

/* ************************************************************************** */

// Pointer to a shell callback
typedef int8_t (*shell_callback_t)(char currentChar);

/*  Registering a shell callback

    The normal behavior of shell_update() is to call getch() to see if the user
    has typed anything. Text goes into the line buffer, control characters..
    control something, and if the user hits ENTER, then the shell will process
    the line buffer and run a shell command if applicable. Since there's no 
    pre-emption, this command just runs to completion and then the shell goes 
    back to processing incoming characters.

    However, when a shell callback is registered, the shell will call getch(),
    check the received character for ctrl+c, and then call the callback. This
    feature allows the creation of long-running and interactive 'programs' that
    run in the background, without blocking the rest of the firmware.

    An interactive shell program has relatively exclusive access to the serial
    port, and can implement any behavior it wants. It can clear the screen and
    print a bunch of text, it can draw ascii art, it create an interactive 
    viewport that lets you use the arrow keys to step through the contents of
    ROM.

    A shell program can request its own termination by returning -1.
*/
extern void shell_register_callback(shell_callback_t callback);

#endif // _SHELL_COMMAND_UTILS_H_