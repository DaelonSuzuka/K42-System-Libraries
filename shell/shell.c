#include "shell.h"
#include "serial_port.h"
#include "shell_command_processor.h"
#include "shell_cursor.h"
#include "shell_history.h"
#include "shell_keys.h"
#include "shell_utils.h"
#include <ctype.h>
#include <string.h>

/* ************************************************************************** */

// stores linebuffer and cursor position
shell_line_t shell;

/* -------------------------------------------------------------------------- */

shell_callback_t shellCallback;

void shell_register_callback(shell_callback_t callback) {
    shellCallback = callback;
}

/*  Program termination

    There's no way to know what state a program was in when we kill it. 

    Therefore, just to be safe we should reset text colors, clear the screen,
    draw a new prompt, and make sure the cursor is visible.
*/
static void terminate_current_program(void) {
    shell_register_callback(NULL);

    reset_text_attributes();
    term_reset_screen();
    term_cursor_set(0, 0);
    draw_shell_prompt();
    term_show_cursor();
}

/* ************************************************************************** */

// set up the entire shell subsystem
void shell_init(uart_config_t *config) {
    serial_port_init(config);

    // initialize shell
    shell_reset_line(shell);
    shell_register_callback(NULL);

    // shell history
    shell_history_init();

    command_processer_init();
}

/* ************************************************************************** */

void process_escape_sequence(key_t key) {
    switch (key.key) {
    default: // unrecognized keys don't do anything
        return;
    case BACKSPACE: // delete one character to the left of the cursor
        if (shell.cursor != 0) {
            move_cursor_left(&shell);
            remove_char_at_cursor(&shell);
        }
        return;
    case ENTER:
        println("");
        if (shell.length > 0) {
            shell_add_terminator_to_line(shell);

            // Push the current line to history before it gets mangled
            shell_history_push();

            // attempt to process the shell buffer
            if (process_shell_command(&shell) == -1) {
                // if there's no valid command, say something
                printf("%s: command not found\r\n", shell.buffer);
            }

            shell_reset_line(shell);

            // only print a new prompt if no callback is registered
            if (!shellCallback) {
                draw_shell_prompt();
            }
            return;
        }
        draw_shell_prompt();
        return;
    case UP:
        shell_history_show_older();
        return;
    case DOWN:
        shell_history_show_newer();
        return;
    case LEFT:
        switch (key.mod) {
        default:
            move_cursor_left(&shell);
            return;
        case CTRL:
            do {
                move_cursor_left(&shell);
            } while (shell.buffer[shell.cursor] != ' ' && shell.cursor > 0);
            return;
        }
    case RIGHT:
        switch (key.mod) {
        default:
            move_cursor_right(&shell);
            return;
        case CTRL:
            do {
                move_cursor_right(&shell);
            } while (shell.buffer[shell.cursor] != ' ' &&
                     shell.cursor < shell.length);
            return;
        }
    case HOME:
        move_cursor_to(&shell, 0);
        return;
    case END:
        move_cursor_to(&shell, shell.length);
        return;
    case DELETE:
        remove_char_at_cursor(&shell);
        return;
    case F7:
        toggle_history_inspection_mode();
        return;
    case F6:
        toggle_key_diagnostics();
        return;
    }
}

/* -------------------------------------------------------------------------- */

void shell_update(void) {
    char currentChar = getch();

    // execute shell callback, if one is registered
    if (shellCallback) {
        // ctrl+c forces the program to terminate
        if (currentChar == 3) {
            terminate_current_program();
            return;
        }
        // execute callback
        if (shellCallback(currentChar) == -1) {
            terminate_current_program();
        }
        return;
    }

    // return early if we haven't rx'd a character
    if (currentChar == 0) {
        return;
    }

    // control characters and escape sequences
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        process_escape_sequence(key);
        return;
    }

    // finally, printable characters
    if (isprint(currentChar)) {
        insert_char_at_cursor(&shell, currentChar);
        return;
    }
}