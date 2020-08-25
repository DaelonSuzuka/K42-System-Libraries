#include "system_clock.h"
#include "peripherals/numerically_controlled_oscillator.h"
#include "peripherals/signal_measurement_timer.h"

/* ************************************************************************** */

void system_clock_init(void) {
    nco_set_pulse_frequency_mode(NCO_MODE_PULSE_FREQUENCY);
    
    // use 500khz MFINTOSC
    // nco_set_clock_source(NCO_CLOCK_SOURCE_MFINTOSC_500);
    // nco_set_incrementor(0x000831);

    // use 32khz LFINTOSC
    nco_set_clock_source(NCO_CLOCK_SOURCE_LFINTOSC);
    nco_set_incrementor(0x008421);
    
    nco_enable();

    smt_set_operation_mode(SMT_MODE_COUNTER);
    smt_set_signal_input(SMT_SIGNAL_INPUT_NCO1);
    smt_clear();

    smt_enable();
    smt_start();

    smt_interrupt_enable();
}

/* -------------------------------------------------------------------------- */

static volatile uint8_t smtOverflowCount;

void __interrupt(irq(SMT1), high_priority) SMT_overflow_ISR() {
    smt_clear_interrupt_flag();

    smtOverflowCount++;
}

/* -------------------------------------------------------------------------- */

/*  Notes on reading the Signal Measurement Timer

    The SMT is a 24 bit counter, whose value is stored in three registers:
    SMT1TMRL
    SMT1TMRH
    SMT1TMRU

    This means that reading the value of the SMT is very far from an atomic
    operation. The processor provides a tool to solve this problem in the form
    of three SMT Captured Period Registers:
    SMT1CPRL
    SMT1CPRH
    SMT1CPRU

    When SMT1STATbits.CPRUP is set, the value of SMT1TMR is snapshotted into
    SMT1CPR, and therefore is safe to read without worrying about corrupted
    data.
*/

system_time_t get_current_time(void) {
    smt_interrupt_disable();
    smt_latch_now();

    system_time_t currentTime = smtOverflowCount;
    currentTime <<= 24;
    currentTime += smt_read();

    smt_interrupt_enable();

    return currentTime;
}

/* -------------------------------------------------------------------------- */

void delay_us(uint16_t microSeconds) {
    while (microSeconds--) {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

void delay_ms(system_time_t milliSeconds) {
    system_time_t startTime = get_current_time();

    while (time_since(startTime) < milliSeconds) {
        // empty loop
    }
}