#ifndef SHELL_HISTORY_ENABLED

/* ************************************************************************** */

// empty function bodies to cleanly disable the module
void shell_history_init(void) {}
void shell_history_wipe(void) {}
void toggle_history_inspection_mode(void) {}
void shell_history_push(void) {}
void shell_history_show_older(void) {}
void shell_history_show_newer(void) {}

#else

/* ************************************************************************** */
#include "shell_history.h"
#include "shell.h"
#include "shell_cursor.h"
#include <stdio.h>
#include <string.h>

/* ************************************************************************** */

// this is disgusting. Thank you pic18 paged memory...
#define HISTORY_LIST                                                           \
    X(0)                                                                       \
    X(1)                                                                       \
    X(2)                                                                       \
    X(3)                                                                       \
    // X(4)                                                                       \
    // X(5)                                                                       \
    // X(6)                                                                       \
    // X(7)                                                                       \
    // X(8)                                                                       \
    // X(9)                                                                       \
    // X(10)                                                                      \
    // X(11)                                                                      \
    // X(12)                                                                      \
    // X(13)                                                                      \
    // X(14)                                                                      \
    // X(15)

#define X(NAME) shell_line_t history_##NAME;
HISTORY_LIST
#undef X

/* -------------------------------------------------------------------------- */

typedef struct {
    shell_line_t *line[SHELL_HISTORY_LENGTH];
    shell_line_t tempLine;
    uint8_t head;
    uint8_t pointer;
    uint8_t length;
    unsigned historyMode : 1;
    unsigned historyInspectionMode : 1;
} shell_history_t;

shell_history_t history;

// clear a line in the shell history
static void clear_history_line(uint8_t line) {
    memset(history.line[line], 0, sizeof(shell_line_t));
}

void shell_history_wipe(void) {
    for (uint8_t i = 0; i < SHELL_HISTORY_LENGTH; i++) {
        clear_history_line(i);
    }

    shell_reset_line(history.tempLine);

    history.head = 0;
    history.pointer = 0;
    history.length = 0;
    history.historyMode = 0;
    history.historyInspectionMode = 0;
}

/* ************************************************************************** */

void shell_history_init(void) {
    // initialize history array with pointers to shell_line_t's
#define X(NAME) history.line[NAME] = &history_##NAME;
    HISTORY_LIST
#undef X

    shell_history_wipe();
}

void toggle_history_inspection_mode(void) {
    history.historyInspectionMode = !history.historyInspectionMode;
    if (history.historyInspectionMode == 1) {
        sh_println("\r\nHistory inspection mode enabled.");
    } else {
        sh_println("\r\nHistory inspection mode disabled.");
    }
    draw_shell_prompt();
}

void inspect_shell_history(void) {
    sh_println("");
    sh_println("-----------------------------------------------");
    sh_println("Printing shell history:");
    printf("History currently has %d entries.\r\n", history.length);
    printf("temp slot: %s\r\n", history.tempLine.buffer);

    for (int8_t i = history.length; i >= 0; i--) {
        uint8_t line = (history.head + i - 1) % SHELL_HISTORY_LENGTH;

        printf("slot #%d: %s", i, history.line[line]->buffer);
        if (i == history.pointer) {
            sh_print(" <--");
        }
        sh_println("");
    }

    printf("current line: %s\r\n", &shell.buffer);
    sh_println("-----------------------------------------------");
    sh_println("");
}

/* -------------------------------------------------------------------------- */
/*

    history.pointer is higher the further into the past we got

    history.pointer is used as an offset from history.head
*/

// push the current line onto the history buffer
// used when we hit the enter key
void shell_history_push(void) {
    // a push means we aren't viewing the middle of the history anymore
    history.pointer = 0;
    history.historyMode = 0;

    // don't save if the current line matches the most recent history entry
    if (strcmp((const char *)history.line[history.head], &shell.buffer[0]) ==
        0) {
        return;
    }

    if (history.historyInspectionMode == 1) {
        inspect_shell_history();
    }

    // Decrement head
    history.head = (history.head - 1) % SHELL_HISTORY_LENGTH;
    if (history.length < SHELL_HISTORY_LENGTH) {
        history.length++;
    }

    // copy the current line into the next slot in the history queue
    memcpy(history.line[history.head], &shell.buffer, sizeof(shell_line_t));
}

// used when we hit the up arrow
void shell_history_show_older(void) {
    // if history is empty then what are we doing?
    if (history.length == 0) {
        return;
    }

    // since we weren't already in history mode, stash the current line
    if (history.historyMode == 0) {
        memcpy(&history.tempLine, &shell.buffer, sizeof(shell_line_t));
        history.historyMode = 1;
    }

    // safely increment the history pointer
    if (history.pointer < SHELL_HISTORY_LENGTH &&
        history.pointer < history.length) {
        history.pointer++;
    }

    uint8_t line = (history.head + history.pointer - 1) % SHELL_HISTORY_LENGTH;

    memcpy(&shell.buffer, history.line[line], sizeof(shell_line_t));

    if (history.historyInspectionMode == 1) {
        inspect_shell_history();
    }
    draw_line(&shell);
    move_cursor_to(&shell, shell.length);
}

// used when we hit the down arrow
void shell_history_show_newer(void) {
    // if history is empty then what are we doing?
    if (history.length == 0) {
        return;
    }

    if (history.historyMode == 0) {
        return;
    }

    history.pointer--;

    // once we reach the newest line, restore the stashed line buffer and quit
    if (history.pointer == 0) {
        history.historyMode = 0;
        memcpy(&shell.buffer, &history.tempLine, sizeof(shell_line_t));
        if (history.historyInspectionMode == 1) {
            inspect_shell_history();
        }
        draw_line(&shell);
        return;
    }

    uint8_t line = (history.head + history.pointer - 1) % SHELL_HISTORY_LENGTH;

    memcpy(&shell.buffer, history.line[line], sizeof(shell_line_t));
    if (history.historyInspectionMode == 1) {
        inspect_shell_history();
    }
    draw_line(&shell);
    move_cursor_to(&shell, shell.length);
}

#endif