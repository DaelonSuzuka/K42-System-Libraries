#include "shell_cursor.h"
#include "shell.h"
#include "shell_utils.h"

/* ************************************************************************** */

void draw_shell_prompt(void) {
    sh_print(SHELL_PROMPT_STRING); //
}

/* -------------------------------------------------------------------------- */

void move_cursor_left(shell_line_t *line) {
    // don't move left past beginning of line
    if (line->cursor == 0) {
        return;
    }

    // adjust cursor value
    line->cursor--;

    // move cursor
    term_cursor_left(1);
}

void move_cursor_right(shell_line_t *line) {
    // don't move right past end of line
    if (line->cursor == shell.length) {
        return;
    }

    // adjust cursor value
    line->cursor++;

    // move cursor
    term_cursor_right(1);
}

void move_cursor_to(shell_line_t *line, uint8_t position) {
    // make sure we don't move past the end of the line
    if (position > line->length) {
        position = line->length;
    }

    term_cursor_home();
    term_cursor_right(SHELL_PROMPT_LENGTH);

    // move the cursor right to the correct spot
    if (position != 0) {
        term_cursor_right(position);
    }

    // adjust cursor value
    line->cursor = position;
}

/* -------------------------------------------------------------------------- */

void draw_line(shell_line_t *line) {
    // clear everything except the prompt
    term_cursor_home();
    term_cursor_right(SHELL_PROMPT_LENGTH);
    term_clear_to_right();

    // reprint existing line
    sh_print(line->buffer);

    // restore the cursor's original position
    move_cursor_to(line, line->cursor);
}

void draw_line_from_cursor(shell_line_t *line) {
    term_clear_to_right();

    // reprint existing line, starting at the cursor position
    sh_print(&line->buffer[line->cursor]);

    // restore the cursor's original position
    move_cursor_to(line, line->cursor);
}

void insert_char_at_cursor(shell_line_t *line, char currentChar) {
    // return early if the buffer is already full
    if (line->length >= SHELL_MAX_LENGTH - SHELL_PROMPT_LENGTH) {
        return;
    }

    // process is easier if cursor is already at end of line
    if (line->cursor == line->length) {
        // print the new char
        printf("%c", currentChar);

        // add the new char to the buffer
        line->buffer[line->cursor] = currentChar;

        // update the length and cursor
        line->length++;
        line->cursor++;

        // and we're done
        return;
    }

    // the cursor won't move, so just update that length
    line->length++;

    // take everything right of the cursor and shift it right one space
    for (uint8_t i = line->length; i >= line->cursor; i--) {
        line->buffer[i + 1] = line->buffer[i];
    }

    // add the new char
    line->buffer[line->cursor] = currentChar;

    draw_line_from_cursor(line);
    move_cursor_right(&shell);
}

void remove_char_at_cursor(shell_line_t *line) {
    // return early if the buffer is already empty
    if (line->length == 0) {
        return;
    }

    // also return early if cursor is at the end of the line
    if (line->cursor == line->length) {
        return;
    }

    // take everything right of the cursor and shift it left one space
    uint8_t i = line->cursor;
    while (line->buffer[i] != 0) {
        line->buffer[i] = line->buffer[i + 1];
        i++;
    }

    line->buffer[i - 1] = 0;
    line->buffer[i] = 0;
    line->buffer[i + 1] = 0;
    line->length--;

    draw_line_from_cursor(line);
}