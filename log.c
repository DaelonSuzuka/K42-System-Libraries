#include "log.h"
#include "serial_port.h"
#include "shell/shell_utils.h"
#include "system_time.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/* ************************************************************************** */

const char *level_names[] = {
    "SILENT", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE",
};

const char *level_colors[] = {
    "\033[1;94m", "\033[1;35m", "\033[1;31m", "\033[1;33m",
    "\033[1;32m", "\033[1;36m", "\033[1;34m",
};

log_database_t logDatabase;

/* -------------------------------------------------------------------------- */

void log_init(void) {
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
    logDatabase.file[logDatabase.numberOfFiles].name = name;
    logDatabase.file[logDatabase.numberOfFiles].levelPtr = levelPtr;
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

    printf("%lu ", get_current_time());

    print_log_level(msgLevel);

    reset_text_attributes();
    printf(" %s:%d: ", file, line);

    reset_text_attributes();
}