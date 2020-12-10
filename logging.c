#include "logging.h"
#include "os/libs/str_len.h"
#include "serial_port.h"
#include "shell/shell_utils.h"
#include "system_time.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#ifndef LOGGING_ENABLED

void logging_init(void) {}
void log_fix_shortnames(void) {}
void log_register__(const char *name, uint8_t *levelPtr) {}
void log_level_edit(uint8_t fileID, uint8_t level) {}
void print_log_level(uint8_t level) {}
void print_log_header(uint8_t msgLevel, const char *file, int line) {}

#else

/* ************************************************************************** */

const char *level_names[] = {
    "SILENT", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE",
};

const char *level_colors[] = {
    "\033[1;37m", "\033[1;35m", "\033[1;31m", "\033[1;33m",
    "\033[1;32m", "\033[1;36m", "\033[1;34m",
};

log_database_t logDatabase;

log_features_t logFeatures = {true, true, true, true, true};

/* -------------------------------------------------------------------------- */

void logging_init(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        logDatabase.file[i].name = NULL;
        logDatabase.file[i].levelPtr = NULL;
    }
    logDatabase.numberOfFiles = 0;
}

void log_register__(const char *name, uint8_t *levelPtr) {
    // make sure we're not double-registering
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        if (!strcmp(name, logDatabase.file[i].name)) {
            return; // file is already registered
        }
    }

    // We're good, register the file
    log_level_t file;
    file.name = name;
    file.levelPtr = levelPtr;

    // fix the shortname pointer
    for (uint8_t j = str_len(file.name); j > 0; j--) {
        if (file.name[j] == '/') {
            file.shortName = &file.name[j + 1];
            break;
        }
    }

    logDatabase.file[logDatabase.numberOfFiles] = file;
    logDatabase.numberOfFiles++;
}

/* ************************************************************************** */

void log_level_edit(uint8_t fileID, uint8_t level) {
    *logDatabase.file[fileID].levelPtr = level;
}

void print_log_level(uint8_t level) {
    printf("%s%-6s", level_colors[level], level_names[level]);
    reset_text_attributes();
}

void print_log_header(uint8_t msgLevel, const char *file, int line) {
    reset_text_attributes();
    if (!logFeatures.printHeader) {
        return;
    }

    if (logFeatures.printTimestamp) {
        printf("%lu ", get_current_time());
    }

    if (logFeatures.printLogLevel) {
        print_log_level(msgLevel);
    }

    reset_text_attributes();

    if (logFeatures.printFileName) {
        if (logFeatures.useShortNames) {
            const char *shortFileName;
            for (uint8_t i = str_len(file); i > 0; i--) {
                if (file[i] == '/') {
                    shortFileName = &file[i + 1];
                    break;
                }
            }
            printf(" %s:%d: ", shortFileName, line);
        } else {
            printf(" %s:%d: ", file, line);
        }
    }

    reset_text_attributes();
}

/* ************************************************************************** */

#include "os/shell/shell.h"
#include "os/shell/shell_command_utils.h"
#include "os/shell/shell_keys.h"
#include "os/shell/shell_utils.h"

/* ************************************************************************** */

static uint8_t oldLogDatabase[MAX_NUMBER_OF_FILES];
static uint8_t newLogDatabase[MAX_NUMBER_OF_FILES];

/* -------------------------------------------------------------------------- */

static uint8_t selectedLine = 0;
static uint8_t selectedLevel = 0;

#define COLUMN_NUMBER 7

void reprint_line(void) {
    term_cursor_home();
    term_cursor_right(COLUMN_NUMBER);
    print_log_level(newLogDatabase[selectedLine]);
    term_cursor_home();
    term_cursor_right(COLUMN_NUMBER);
}

/* -------------------------------------------------------------------------- */

// This should be the number of newlines printed by print_logedit_header()
#define HEADER_HEIGHT 5

void print_logedit_header(void) {
    println("-----------------------------------------------");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    if (logFeatures.useShortNames) {
        println(" #   | level  | file name");
    } else {
        println(" #   | level  | path/to/file");
    }
    println("-----------------------------------------------");
}

void print_managed_log_table(void) {
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-2d", (int)i);
        print(" | ");
        print_log_level(newLogDatabase[i]);
        print(" | ");
        if (logFeatures.useShortNames) {
            println(logDatabase.file[i].shortName);
        } else {
            println(logDatabase.file[i].name);
        }
    }
}

void print_logedit_footer(void) {
    println("-----------------------------------------------");
    for (uint8_t i = 0; i < NUMBER_OF_LOG_LEVELS; i++) {
        print_log_level(i);
        print(" ");
    }
    println("");
    println("-----------------------------------------------");

    // usage instructions
    //! These are disabled to help logedit fit on page in the terminal
    // println("press f5 to refresh list");
    // println("press ENTER to save and exit");
    // println("press ESC to exit without saving");
    // println("press ctrl+c to terminate logedit");
}

void draw_logedit(void) {
    // wipe the screen
    term_reset_screen();
    term_cursor_set(0, 0);

    // draw charts and graphs
    print_logedit_header();
    print_managed_log_table();
    print_logedit_footer();

    // set the cursor to the first log level
    term_cursor_set(0, 0);
    term_cursor_down(HEADER_HEIGHT);
    selectedLine = 0;
    selectedLevel = 0;

    // highlight the first line
    invert_text_attribute();
    reprint_line();
}

/* -------------------------------------------------------------------------- */

void up_one_line(void) {
    reprint_line();
    selectedLine--;
    selectedLevel = newLogDatabase[selectedLine];
    term_cursor_up(1);
    invert_text_attribute();
    reprint_line();
}

void down_one_line(void) {
    reprint_line();
    selectedLine++;
    selectedLevel = newLogDatabase[selectedLine];
    term_cursor_down(1);
    invert_text_attribute();
    reprint_line();
}

void increase_level(void) {
    selectedLevel++;
    newLogDatabase[selectedLine] = selectedLevel;
    invert_text_attribute();
    reprint_line();
}

void decrease_level(void) {
    selectedLevel--;
    newLogDatabase[selectedLine] = selectedLevel;
    invert_text_attribute();
    reprint_line();
}

int8_t logedit_keys(key_t key) {
    switch (key.key) {
        default:
            return 0;
        case UP:
            if (selectedLine > 0) {
                up_one_line();
            }
            return 0;
        case DOWN:
            if (selectedLine < logDatabase.numberOfFiles - 1) {
                down_one_line();
            }
            return 0;
        case LEFT:
            if (selectedLevel > 0) {
                decrease_level();
            }
            return 0;
        case RIGHT:
            if (selectedLevel < 6) { // todo: magic number
                increase_level();
            }
            return 0;
        case F1:
            logFeatures.printTimestamp = !logFeatures.printTimestamp;
            return 0;
        case F2:
            logFeatures.printLogLevel = !logFeatures.printLogLevel;
            return 0;
        case F3:
            logFeatures.printFileName = !logFeatures.printFileName;
            return 0;
        case F4:
            logFeatures.useShortNames = !logFeatures.useShortNames;
            draw_logedit();
            return 0;
        case F5:
            draw_logedit();
            return 0;
        case ENTER:
            for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
                *logDatabase.file[i].levelPtr = newLogDatabase[i];
            }
            return -1;
        case ESCAPE:
            for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
                *logDatabase.file[i].levelPtr = oldLogDatabase[i];
            }
            return -1;
    }
}

/* ************************************************************************** */

// loop - executed on every key press
// must have a return to indicate desire to terminate program
// if the correct state to do that comes about during the last keypress
int8_t logedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        return logedit_keys(key);
    }

    return 0;
}

// setup
void sh_logedit(int argc, char **argv) {
    // make a backup of the existing log database
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        oldLogDatabase[i] = *logDatabase.file[i].levelPtr;
    }
    // use that backup to initialize our new working copy
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        newLogDatabase[i] = oldLogDatabase[i];
    }
    // set the "real" log data to all silent
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        *logDatabase.file[i].levelPtr = L_SILENT;
    }

    term_hide_cursor();
    draw_logedit();

    shell_register_callback(logedit_callback);
}

#endif // #ifdef LOGGING_ENABLED