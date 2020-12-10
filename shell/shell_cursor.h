#ifndef _SHELL_CURSOR_H_
#define _SHELL_CURSOR_H_

#include "shell.h"
#include <stdint.h>

/* ************************************************************************** */

extern void draw_shell_prompt(void);

/* -------------------------------------------------------------------------- */

// move cursor left one space, stopping at the prompt
extern void move_cursor_left(shell_line_t *line);

// move cursor right one space, stopping at the end of the line
extern void move_cursor_right(shell_line_t *line);

// move cursor to the desired absolute position
extern void move_cursor_to(shell_line_t *line, uint8_t position);

/* -------------------------------------------------------------------------- */

// erase the line and reprint it from line
extern void draw_line(shell_line_t *line);

// erase the line after the cursor and then reprint
extern void draw_line_from_cursor(shell_line_t *line);

// add a character to the line at the cursor, adjusting the line as necessary
extern void insert_char_at_cursor(shell_line_t *line, char currentChar);

// delete the character at the cursor, adjusting the line as necessary
extern void remove_char_at_cursor(shell_line_t *line);

#endif