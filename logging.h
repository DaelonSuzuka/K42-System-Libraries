#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdbool.h>
#include <stdint.h>

#include "serial_port.h"

/* ************************************************************************** */

#define NUMBER_OF_LOG_LEVELS 7

// Available logging levels
typedef enum {
    L_SILENT,
    L_FATAL,
    L_ERROR,
    L_WARN,
    L_INFO,
    L_DEBUG,
    L_TRACE,
} log_levels_t;

/* ************************************************************************** */
/*  log_register__() registers a file with the log manager

    Registering a file allows its log level to be edited at runtime using the
    logedit shell program.
*/
//! Do not call directly. Use log_register() wrapper macro defined below.
extern void log_register__(const char *name, uint8_t *level);

/*  log_header() prints nicely formatted log message headers

    <timestamp> <LEVEL> <path/to/source/file/c:line#>: [custom message]
    example:
    60902l TRACE src/tuning.c:500: full_tune
*/
//! Do not call directly. Use LOG_XXXX() wrapper macros defined below.
extern void print_log_header(uint8_t msgLevel, const char *file, int line);

/* ************************************************************************** */

#ifdef LOGGING_ENABLED

#define log_register() log_register__(__FILE__, &LOG_LEVEL)

#define ON_TRACE(BLOCK)                                                        \
    if (L_TRACE <= LOG_LEVEL) {                                                \
        BLOCK                                                                  \
    }
#define ON_DEBUG(BLOCK)                                                        \
    if (L_DEBUG <= LOG_LEVEL) {                                                \
        BLOCK                                                                  \
    }
#define ON_INFO(BLOCK)                                                         \
    if (L_INFO <= LOG_LEVEL) {                                                 \
        BLOCK                                                                  \
    }
#define ON_WARN(BLOCK)                                                         \
    if (L_WARN <= LOG_LEVEL) {                                                 \
        BLOCK                                                                  \
    }
#define ON_ERROR(BLOCK)                                                        \
    if (L_ERROR <= LOG_LEVEL) {                                                \
        BLOCK                                                                  \
    }
#define ON_FATAL(BLOCK)                                                        \
    if (L_FATAL <= LOG_LEVEL) {                                                \
        BLOCK                                                                  \
    }

#define LOG_TRACE(BLOCK)                                                       \
    if (L_TRACE <= LOG_LEVEL) {                                                \
        print_log_header(L_TRACE, __FILE__, __LINE__);                         \
        BLOCK                                                                  \
    }
#define LOG_DEBUG(BLOCK)                                                       \
    if (L_DEBUG <= LOG_LEVEL) {                                                \
        print_log_header(L_DEBUG, __FILE__, __LINE__);                         \
        BLOCK                                                                  \
    }
#define LOG_INFO(BLOCK)                                                        \
    if (L_INFO <= LOG_LEVEL) {                                                 \
        print_log_header(L_INFO, __FILE__, __LINE__);                          \
        BLOCK                                                                  \
    }
#define LOG_WARN(BLOCK)                                                        \
    if (L_WARN <= LOG_LEVEL) {                                                 \
        print_log_header(L_WARN, __FILE__, __LINE__);                          \
        BLOCK                                                                  \
    }
#define LOG_ERROR(BLOCK)                                                       \
    if (L_ERROR <= LOG_LEVEL) {                                                \
        print_log_header(L_ERROR, __FILE__, __LINE__);                         \
        BLOCK                                                                  \
    }
#define LOG_FATAL(BLOCK)                                                       \
    if (L_FATAL <= LOG_LEVEL) {                                                \
        print_log_header(L_FATAL, __FILE__, __LINE__);                         \
        BLOCK                                                                  \
    }

#else // #ifdef LOGGING_ENABLED

#define log_register()
#define LOG_TRACE(BLOCK)
#define LOG_DEBUG(BLOCK)
#define LOG_INFO(BLOCK)
#define LOG_WARN(BLOCK)
#define LOG_ERROR(BLOCK)
#define LOG_FATAL(BLOCK)

#endif // #ifdef LOGGING_ENABLED

/* ************************************************************************** */
/*  log_level_t
    This is essentially a key:value object that represents the current logging
    level of each registered file
*/
typedef struct {
    const char *name;
    uint8_t *levelPtr;
    const char *shortName;
} log_level_t;

/*  log_settings_t

    Expirimental new layout for log level controls. Bitfield will allowing
    toggling individual logging levels.

    Here's a new log levels enum to go with it:

    typedef enum {
        L_SILENT = 0x00,
        L_FATAL = 0x01,
        L_ERROR = 0x02,
        L_WARN = 0x04,
        L_INFO = 0x08,
        L_DEBUG = 0x10,
        L_TRACE = 0x20,
    } log_levels_t;

    typedef struct {
        union {
            struct {
                unsigned fatal : 1;
                unsigned error : 1;
                unsigned warn : 1;
                unsigned info : 1;
                unsigned debug : 1;
                unsigned trace : 1;
            };
            uint8_t level;
        };
        const char *fileName;
        const char *fullFileName;
    } log_settings_t;
*/

#ifdef LOGGING_ENABLED
#define MAX_NUMBER_OF_FILES 20
#else
#define MAX_NUMBER_OF_FILES 1
#endif

typedef struct {
    log_level_t file[MAX_NUMBER_OF_FILES];
    uint8_t numberOfFiles;
} log_database_t;

extern log_database_t logDatabase;

typedef struct LogFeatures {
    unsigned useShortNames : 1;
    unsigned printHeader : 1;
    unsigned printTimestamp : 1;
    unsigned printLogLevel : 1;
    unsigned printFileName : 1;
} log_features_t;

extern log_features_t logFeatures;

/* ************************************************************************** */

// setup
extern void logging_init(void);

// modify a the log level for the given fileID
extern void log_level_edit(uint8_t fileID, uint8_t level);

// modify a the log level for the given file shortname
extern void set_log_level(const char *filename, uint8_t level);

// future api
extern void push_log_level(const char *filename, uint8_t level);
extern void pop_log_level(const char *filename);

// print the specified log level
extern void print_log_level(uint8_t level);

/* ************************************************************************** */

#endif