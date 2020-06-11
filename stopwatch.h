#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <stdint.h>
#include <stdio.h>

/* ************************************************************************** */

/*  Notes on using stopwatch module:

    Measuring a time longer than 1 overflow of Timer0 is going to involve
    several interrupt triggers. Be careful using either stopwatch inside
    critical sections.

    Some care must be taken to prevent multiple uses of the stopwatch from
    overlapping. Timers are a limited resource, and obviously cannot be used for
    multiple tasks at once. Any call to xx_stopwatch_begin() will overwrite any
    active stopwatches, corrupting any possible readings.

    There is currently no mechanism in place to detect or avoid these
    conditions, since the stopwatch is NOT intended for use in production code.

    Due to the above limitation, ms_multiwatch has been introduced. Because the
    multiwatch uses the system tick instead of a hardware timer, the equivalent
    of several ms_stopwatches can be active at once. It is up to the user to
    avoid collisions between multiple stopwatchID values.
*/

/* ************************************************************************** */

// Setup
extern void stopwatch_init(void);

/* -------------------------------------------------------------------------- */

// Microsecond stopwatch functions, counts elapsed time in us +/- 1%
extern void us_stopwatch_begin(void);
extern uint32_t us_stopwatch_end(void);

// utility macros
#define us_stopwatch_print() printf("%lu uS", us_stopwatch_end())
#define us_stopwatch_println() printf("%lu uS\r\n", us_stopwatch_end())

/* -------------------------------------------------------------------------- */

// Millisecond stopwatch functions, counts elapsed time in ms +/- 1ms
extern void ms_stopwatch_begin(void);
extern uint32_t ms_stopwatch_end(void);

// utility macros
#define ms_stopwatch_print() printf("%lu mS", ms_stopwatch_end())
#define ms_stopwatch_println() printf("%lu mS\r\n", ms_stopwatch_end())

/* -------------------------------------------------------------------------- */

// Millisecond multiwatch functions, counts elapsed time in ms +/- 1ms
// Can run multiple, concurrent multiwatches by providing unique ID numbers
#define CONCURRENT_MULTIWATCHES 16
extern void ms_multiwatch_begin(uint8_t stopwatchID);
extern uint32_t ms_multiwatch_end(uint8_t stopwatchID);

// utility macros
#define ms_multiwatch_print(stopwatchID)                                       \
    printf("%lu mS", ms_multiwatch_end(stopwatchID))
#define ms_multiwatch_println(stopwatchID)                                     \
    printf("%lu mS\r\n", ms_multiwatch_end(stopwatchID))

#endif // _STOPWATCH_H_