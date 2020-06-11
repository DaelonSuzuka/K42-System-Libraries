#ifndef _LOG_MACROS_H_
#define _LOG_MACROS_H_

#include <stdbool.h>
#include <stdint.h>

#include "log.h"
#include "serial_port.h"

/* ************************************************************************** */

// Comment out this line to cleanly disable ALL logging macros
#define LOGGING_ENABLED

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

/*  log_register() is a wrapper macro for log_register__()

    This macro will register your file with the log manager, so that its log
    level can be edited at runtime using the logedit shell program. Registration
    requires a filename and a pointer to the file-scope variable representing
    the current logging level.

    For ease-of-use, this macro fills in the arguments for you. It uses the
    compiler-provided __FILE__ macro, which expands to a string literal of the
    current filename and line number (ex: "src/tuning.c:500"). It also assumes
    that a uint8_t variable named LOG_LEVEL has been defined at the file scope.
*/
#define log_register() log_register__(__FILE__, &LOG_LEVEL)

//! The following stuff is pretty weird, so pay attention!
/*  LOG_XXXX() wrappers

    On their own, these macros merely check if they're supposed to execute, and
    print formatted log headers. You have to provide your own block of code to
    format and print your log message. Your block of code is not limited to
    print statements. Any statements will work properly in these macros as long
    as they have semicolons and are collectively wrapped in a pair of curly
    braces.

    If this sounds unusual, please remember that macros are not C functions,
    even if they look like them. The C Preprocessor only performs text
    substitutions, simple find-and-replace.

    These macros REQUIRE that LOG_LEVEL be defined INSIDE the file that the
    macro is being called from.

    header format:
    <timestamp> <LEVEL> <path/to/source/file/c:line#>: [custom message]
    example:
    60902l TRACE src/adc.c:246: adc_read

    single statements:
    LOG_TRACE({ println("adc_read"); });
    LOG_INFO({ printf("found valid records at: %d", address); });

    multiple statements:
    LOG_INFO({
        print("Registering: ");
        print_event(&newEvent);
    });

    expansion of previous example:
    if (L_TRACE <= LOG_LEVEL) {
        log_header(L_TRACE, __FILE__, __LINE__);
        print("Registering: ");
        print_event(&newEvent);
    }

*/
#define LOG_TRACE(LOG_MESSAGE_BLOCK)                                           \
    if (L_TRACE <= LOG_LEVEL) {                                                \
        print_log_header(L_TRACE, __FILE__, __LINE__);                         \
        LOG_MESSAGE_BLOCK                                                      \
    }
#define LOG_DEBUG(LOG_MESSAGE_BLOCK)                                           \
    if (L_DEBUG <= LOG_LEVEL) {                                                \
        print_log_header(L_DEBUG, __FILE__, __LINE__);                         \
        LOG_MESSAGE_BLOCK                                                      \
    }
#define LOG_INFO(LOG_MESSAGE_BLOCK)                                            \
    if (L_INFO <= LOG_LEVEL) {                                                 \
        print_log_header(L_INFO, __FILE__, __LINE__);                          \
        LOG_MESSAGE_BLOCK                                                      \
    }
#define LOG_WARN(LOG_MESSAGE_BLOCK)                                            \
    if (L_WARN <= LOG_LEVEL) {                                                 \
        print_log_header(L_WARN, __FILE__, __LINE__);                          \
        LOG_MESSAGE_BLOCK                                                      \
    }
#define LOG_ERROR(LOG_MESSAGE_BLOCK)                                           \
    if (L_ERROR <= LOG_LEVEL) {                                                \
        print_log_header(L_ERROR, __FILE__, __LINE__);                         \
        LOG_MESSAGE_BLOCK                                                      \
    }
#define LOG_FATAL(LOG_MESSAGE_BLOCK)                                           \
    if (L_FATAL <= LOG_LEVEL) {                                                \
        print_log_header(L_FATAL, __FILE__, __LINE__);                         \
        LOG_MESSAGE_BLOCK                                                      \
    }

#else // #ifdef LOGGING_ENABLED

#define log_register()
#define LOG_TRACE(LOG_MESSAGE_BLOCK)
#define LOG_DEBUG(LOG_MESSAGE_BLOCK)
#define LOG_INFO(LOG_MESSAGE_BLOCK)
#define LOG_WARN(LOG_MESSAGE_BLOCK)
#define LOG_ERROR(LOG_MESSAGE_BLOCK)
#define LOG_FATAL(LOG_MESSAGE_BLOCK)
#endif

/* ************************************************************************** */

#endif

/*  LOG_XXXX() wrappers

    These macros make it easier to write compact, manageable log messages. This
    style of log macro began life as a desperate attempt to work around missing
    features in XC8 v1.45, namely variadic macros and the entire vprintf family.

    * Disclaimer *

    These only work due to a dirty dirty trick, and it is very shameful that I
    have done this. This sort of behavior is STRONGLY DISCOURAGED for a myriad
    of reasons, including but not limited to: readability, maintainability,
    portability, aesthetics, and the bounds of good taste. Please consult your
    ombudsman before using this library.

    Now that the self-flagellation is out of the way: I've actually taken quite
    a liking to my dirty dirty trick, and I don't plan to stop using it in the
    foreseeable future.

    * What I was trying to do *

    I needed a logging library with multiple message levels and per-file level
    controls. It also needed to be easier to use than my previous logging
    library that required two pound signs per invocation. Changing the logging
    level in a file required editing a large block of preprocessor statements in
    a totally different file and recompiling and reuploading the program. The
    only truly redeeming factor was that when logging statements were turned
    off, they were OFF. Since it was all controlled by the pre-processor,
    messages that weren't active didn't even make it to the compiler. This
    allowed the creation of a persistent suite of debug messages(aka not
    reinventing the wheel when a new bug surfaced) with the ability to force
    them all off for production builds.

    Here's an example of the old style:

    #if LOG_LEVEL_STARTUP >= LOG_EVENTS
        println("Hello!");
    #endif

    Doesn't that just make your eyes bleed? Imagine writing hundreds of these to
    track complex state changes through big 800 line long algorithms with fat
    helpings of sensor data. It worked, but it sure did suck.

    Fast forwarding to the present, the replacement logging library had to match
    and exceed the previous library. Originally that just meant better syntax
    and managing the logging level inside the file you're logging from. It was
    soon obvious that a new, killer feature was possible: changing log levels
    at runtime using the shell.

    * What I should have done *

    The 'correct', 'traditional', or 'idiomatic' way to make a library like this
    is to use some combination of variadic macros and vprintf to make a custom
    print function that has a built-in guard to only print if the appropriate
    criteria are met. This is C89 on a PIC18, and we are quite a long distance
    from 'correct'. Instead, we have... regular macros and regular function
    calls. As Tim Gunn would say, "Make it work!"

    * What I did *

    My initial inspiration for this library was a library called log.c, authored
    by GitHub user rxi(https://github.com/rxi/log.c). I liked its clean output,
    with timestamps, color coded log levels, and filenames/linenumbers. I also
    liked that it's just two files that you drop into your project and go. My
    compiler liked that it was written in C99 and used both variadic macros and
    vfprintf(). Wait. Fuck.

    C89 and PIC18 didn't leave a lot of options for recreating rxi's "printf
    with extra parts" style. I knew I liked the log.c output. I knew I wanted
    the ability to have single line log messages. After several days thinking
    and reading about vaious kinds of preprocessor abuse, I got an idea. An
    awful idea. I had a wonderful, awful idea. I don't need to handle variable
    macro arguments. I don't even need to handle multiple macro arguments.

    * The dirty, dirty trick *

    The preprocessor will happily handle any amount of garbage wrapped in curly
    braces as if it were a single argument.

    Thats it. It doesn't sound that bad on its own. Let's look at an example:

    LOG_INFO({ println("Hello!"); });

    Note the extra semicolon inside the parens. Note the curly braces stacked up
    against the parens. You are looking at genuine, certifiable preprocesor
    abuse, people. I argued with myself whether it was worth it. I called in
    other engineers and had lengthy code reviews about the placement of those
    three extra C-syntax destroying characters. We decided not to veto the idea,
    because it was still easier to use than the old logging library. Let's try
    it for now and revisit the idea with some usage examples.

    * The... Redemption? *

    So I used it. It worked. Logging messages when you want them, silence when
    you don't. The shell-based log level editing was actually a gamechanger.

    So it worked. Boring. Where's the redemption?

    Reread the description of the dirty dirty trick. Get it yet? "Any amount of
    garbage wrapped in curly braces." The log macro doesn't want a string as an
    input, or even a printf-style format string like rxi's log.c. The input is
    an arbitrary block of code.

    Want to print a string? No problem:

    LOG_TRACE({ println("function_name"); });

    Print the contents of variables? Easy:

    LOG_DEBUG({
        printf("channel: %d, maxVoltage: %d\r\n", channel, maxVoltage[channel]);
    });

    Use special functions to print custom data visualizations? Child's play:

    LOG_INFO({
        println("verifying:");
        print_flash_buffer(address, buffer);
    });

    Have a piece of data that only needs to be updated if you're printing log
    messages? We can do that too:

    LOG_INFO({
        printf("event is ready, last event was %lu ago\r\n",
               (uint32_t)time_since(previousTime));
        previousTime = currentTime;
    });

    How about conditional execution of diagnostic tools? Fuggedaboutit:

    LOG_DEBUG({ us_stopwatch_begin(); });
    function_you_want_to_measure();
    LOG_DEBUG({
        us_stopwatch_end();
        printf("%lu microseconds", us_stopwatch_read());
    });
*/