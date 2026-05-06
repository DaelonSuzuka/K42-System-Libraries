#ifndef _SYSTEM_TIME_H_
#define _SYSTEM_TIME_H_

#include <stdint.h>

/* ************************************************************************** */
/*  Notes on using the System Tick module

    This module counts the system's uptime in milliseconds. It functions
    similarly to the millis() function from the Arduino platform.

    There are several differences between millis() and get_current_time():

    millis() requires an interrupt to fire every millisecond, and an ISR that
    increments a 4-byte-long uint32_t. This is a very quick ISR, but it's less
    than ideal that this ISR will need to be running all the time, behind any
    other operations you intend to do on your system. get_current_time() uses no
    interrupt, so it still counts time during critical sections of code and will
    never contribute to interrupt-based timer bugs.

    This module uses several hardware features of the PIC18 Q41 family. One
    hardware timer (the NCO) is configured to generate a ~1kHz pulse train,
    which the Signal Measurement Timer (SMT) counts. The SMT is a 24-bit counter
    that increments once per millisecond, completely in the background. No
    interrupts are necessary to count time.

    An 8-bit overflow counter extends the 24-bit SMT to a 32-bit millisecond
    counter, giving a range of ~49.7 days before overflow. The ISR that
    increments this counter fires only every ~4.66 hours (when the SMT
    overflows).

    The SMT has another special feature that assists with read operations.
    Manipulating a 24-bit value on an 8-bit system takes several operations,
    potentially causing data corruption if the register value changes in the
    middle of a read. The SMT1STAT register has a bit that causes the SMT count
    register to be copied to a buffer, which can then be read from without
    worrying about atomic access.

    Maximum countable time:
    UINT32_MAX = 4,294,967,296 ticks until overflow

    Each tick = 1 millisecond:
    4,294,967,296 / 1000 = 4,294,967.296 seconds
    4,294,967 / 60 = 71,582.7 minutes
    71,582 / 60 = 1,193.0 hours
    1,193 / 24 = ~49.7 days until overflow

*/
/* ************************************************************************** */

typedef uint32_t system_time_t;

/* ************************************************************************** */

// setup
extern void system_time_init(void);

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