#include "stopwatch.h"
#include "peripherals/timer.h"
#include "serial_port.h"
#include "system_clock.h"

/* ************************************************************************** */

// Comment this out to prevent stopwatch_self_test() from compiling
// #define STOPWATCH_SELF_TEST_ENABLED

// Test function used to calibrate the stopwatch
#ifdef STOPWATCH_SELF_TEST_ENABLED
void stopwatch_self_test(void);
#endif

/* ************************************************************************** */

void stopwatch_init(void) {
    // set timer0 to overflow in exactly 1uS
    T0CON0bits.MD16 = 1; // 16 bit mode
    timer0_clock_source(TMR0_CLK_FOSC4);
    timer0_prescale(TMR_PRE_1_16);

#ifdef STOPWATCH_SELF_TEST_ENABLED
    stopwatch_self_test();
#endif
}

/* -------------------------------------------------------------------------- */
/*  Notes on us_stopwatch_ISR() and the Microsecond Stopwatch

    This function is an Interrupt Vector Table compatible ISR to respond to the
    TMR0 interrupt signal. This signal is generated whenever timer0 overflows
    from 0xffff to 0x0000. The timer DOES automatically overflow, and therefore
    does not need to be reset from in side the ISR. The Timer 0 Interrupt Flag
    does need be cleared by software.

    The purpose of this interrupt is to allow the microsecond stopwatch to count
    times that are too large to fit in the 16 bits of timer0. When timer0
    overflows(hits 0xffff and resets to 0x0000), us_stopwatch_ISR() will add
    0xffff to stopwatchCount, acting as a pseudo extension to the timer0
    counter. When us_stopwatch_end() is called, timer0 is added to this
    accumulated total and printed to the debug console.

    Timer0 is configured in 16 bit mode, its clock source is set to FOSC/4, or
    16MHz, and its prescaler is set to 1:16. It increments by 1 every 16 clock
    cycles, or at 1MHz. 1MHz = 1,000,000Hz, each timer tick is therefore
    1/1,000,000th of a second, or exactly 1uS.
*/

volatile static uint32_t stopwatchCount;

void __interrupt(irq(TMR0), high_priority) us_stopwatch_ISR(void) {
    timer0_IF_clear();

    stopwatchCount += UINT16_MAX;
}

void us_stopwatch_begin(void) {
    // Clear old stuff
    stopwatchCount = 0;
    timer0_IF_clear();
    timer0_clear();

    // Enable interrupt and engage
    timer0_interrupt_enable();
    timer0_start();
}

uint32_t us_stopwatch_end(void) {
    // Disable interrupts and stop
    timer0_stop();
    timer0_interrupt_disable();

    // calculate final value and return
    stopwatchCount += timer0_read();
    return stopwatchCount;
}

/* -------------------------------------------------------------------------- */
/*  Notes on the Millisecond Stopwatch

    This stopwatch does not require a hardware timer. Instead, it uses the
    system tick to count elapsed time between calls to begin() and end().

    Due to the single static variable ms_startTime, this stopwatch breaks when
    multiple pairs of begin() and end() calls overlap.
*/
static system_time_t ms_startTime = 0;

void ms_stopwatch_begin(void) { ms_startTime = get_current_time(); }

uint32_t ms_stopwatch_end(void) { return time_since(ms_startTime); }

/* -------------------------------------------------------------------------- */
/*  Notes on the Millisecond Multiwatch

    The multiwatch is a modified ms_stopwatch that supports multiple watches
    "running" simultaneously. No additional hardware resources are needed, only
    minor additional RAM for storing an array of ms_startTime variables.

    The user has to manage thier own stopwatchIDs.
*/
static system_time_t ms_multiwatchStartTimes[CONCURRENT_MULTIWATCHES];

void ms_multiwatch_begin(uint8_t stopwatchID) {
    // stash the start_time
    ms_multiwatchStartTimes[stopwatchID] = get_current_time();
}

uint32_t ms_multiwatch_end(uint8_t stopwatchID) {
    return time_since(ms_multiwatchStartTimes[stopwatchID]);
}

/* ************************************************************************** */

#ifdef STOPWATCH_SELF_TEST_ENABLED
uint16_t testValues[] = {1, 5, 10, 100, 200, 500, 1000, 5000, 10000, 20000};

void stopwatch_self_test(void) {
    println("");
    println("-----------------------------------------------");
    println("microsecond delays with microsecond stopwatch");

    for (uint8_t i = 0; i < 10; i++) {
        printf("%u uS: ", testValues[i]);
        us_stopwatch_begin();
        delay_us(testValues[i]);
        us_stopwatch_println();
    }

    println("");
    println("-----------------------------------------------");
    println("millisecond delays with microsecond stopwatch");

    for (uint8_t i = 0; i < 10; i++) {
        printf("%u mS: ", testValues[i]);
        // Since delay_ms counts milliseconds as the system ticks increment,
        // it's necessary to align the start of the microsecond stopwatch with
        // the start of a system tick by waiting for a few system ticks to go
        // by.
        delay_ms(2); // Wait for the rest of the current millisecond to finish, 
                     // tick to increment, and for another full tick to go by
        us_stopwatch_begin();
        delay_ms(testValues[i]);
        us_stopwatch_println();
    }

    println("");
    println("-----------------------------------------------");
    println("millisecond delays with millisecond stopwatch");

    for (uint8_t i = 0; i < 10; i++) {
        printf("%u mS: ", testValues[i]);
        ms_stopwatch_begin();
        delay_ms(testValues[i]);
        ms_stopwatch_println();
    }
}

#endif