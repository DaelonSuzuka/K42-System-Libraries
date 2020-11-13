#ifndef _LOG_H_
#define _LOG_H_

#include <stdbool.h>
#include <stdint.h>

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

typedef struct LogFeatures{
    unsigned useShortNames : 1;
    unsigned printHeader : 1;
    unsigned printTimestamp : 1;
    unsigned printLogLevel : 1;
    unsigned printFileName : 1;
} log_features_t;

extern log_features_t logFeatures;

/* ************************************************************************** */

// setup
extern void log_init(void);

// modify a the log level for the given file
extern void log_level_edit(uint8_t fileID, uint8_t level);

// print the specified log level
extern void print_log_level(uint8_t level);

/* ************************************************************************** */

#endif