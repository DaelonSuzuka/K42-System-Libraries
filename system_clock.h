#ifndef _SYSTEM_CLOCK_H_
#define _SYSTEM_CLOCK_H_

#include <stdint.h>

/* ************************************************************************** */
/*  Notes on using the System Clock module

    This module counts the system's uptime in milliseconds. It functions
    similarly to the millis() function from the Arduino platform.

    Unlike millis(), this system clock module uses 

    This module uses several hardware features of the K42. One hardware timer of
    any variety needs to be configured to overflow every millisecond.

    This timer is then configured as the clock source of the Signal Measurement
    Timer, a special 24-bit counter. This causes the SMT to increment once every
    millisecond, completely in the background. No interrupts are necessary to
    count time.

    The SMT has another special feature that assists with read operations.
    Manipulating a 24-bit value on an 8-bit system takes several operations,
    potentially causing data corruption if the register value changes in the
    middle of a read. The SMT1STAT register has a bit that causes the SMT count
    register to be copied to a buffer, which can then be read from without
    worrying about atomic access.

    Maximum countable time:
    UINT24_MAX = 16,777,216 ticks until overflow

    Each tick = 1 millisecond:
    16,777,216 / 1000 = 16,777.216 seconds
    16,777 / 60 = 279.6 minutes
    279 minutes / 60 = 4.6 hours until overflow

*/
/* ************************************************************************** */

typedef uint32_t system_time_t;

/* ************************************************************************** */

// setup
extern void system_clock_init(void);

/* -------------------------------------------------------------------------- */

// returns the number of milliseconds since boot
extern system_time_t get_current_time(void);

//  time_since() returns the time 
#define time_since(startTime) (get_current_time() - startTime)

/* -------------------------------------------------------------------------- */

// delay_us() uses NOPs to wait n microseconds, to an accuracy of +-2%
extern void delay_us(uint16_t microSeconds);

/*  delay_ms() uses the system tick to wait between n-1 and n milliseconds.
    This error is caused by the 1ms resolution of the systick. delay_ms() can
    start anywhere in the 'current' ms. Calling delay_ms() with arguments less
    than 10 can result in ~(1/(n+1))% error, as the jitter becomes a larger and
    larger fraction of the desired wait time.

    If a high-accuracy delay is required, please use delay_us() instead.
*/
extern void delay_ms(system_time_t milliSeconds);

#endif