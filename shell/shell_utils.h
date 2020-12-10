#ifndef _SHELL_UTILS_H_
#define _SHELL_UTILS_H_

/* ************************************************************************** */

#include "shell.h"
#include <stdio.h>

/* ************************************************************************** */
// terminal control escape sequences

// reset text colors to default (no background, white text)
#define reset_text_attributes() sh_print("\033[0;37;40m")

// swap text and background colors
#define invert_text_attribute() sh_print("\033[7m");

/* -------------------------------------------------------------------------- */

// clear the entire screen and move the cursor to 0,0
#define term_reset_screen() sh_print("\033[2J")

// clear the line to the right of the cursor
#define term_clear_to_right() sh_print("\033[0K")

/* -------------------------------------------------------------------------- */

#define term_hide_cursor() sh_print("\033[?25l")
#define term_show_cursor() sh_print("\033[?25h")

// move the cursor to the given position
#define term_cursor_set(x, y) printf("\033[%d;%dH", y, x)

// move the cursor left a ton. The terminal won't let it go past the edge.
#define term_cursor_home() sh_print("\033[100D")

// move the cursor in some direction
#define term_cursor_up(distance) printf("\033[%dA", distance)
#define term_cursor_down(distance) printf("\033[%dB", distance)
#define term_cursor_right(distance) printf("\033[%dC", distance)
#define term_cursor_left(distance) printf("\033[%dD", distance)

/* ************************************************************************** */

#endif