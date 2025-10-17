#include "system_time.h"
#include "os/shell/shell_command_utils.h"
#include "peripherals/numerically_controlled_oscillator.h"
#include "peripherals/signal_measurement_timer.h"

/* ************************************************************************** */

// forward declaration
extern void sh_clockmon(int argc, char **argv);
extern void sh_uptime(int argc, char **argv);

void system_time_init(void) {
    nco1_set_pulse_frequency_mode(NCO_MODE_PULSE_FREQUENCY);

    // TODO: don't hardcode this, have the project set it in system.c
    // use 500khz MFINTOSC
    nco1_set_clock_source(NCO_CLOCK_SOURCE_MFINTOSC_500);
    nco1_set_incrementor(0x000831);

    // use 32khz LFINTOSC
    // nco1_set_clock_source(NCO_CLOCK_SOURCE_LFINTOSC);
    // nco1_set_incrementor(0x008421);

    nco1_enable();

    smt_set_operation_mode(SMT_MODE_COUNTER);
    smt_set_signal_input(SMT_SIGNAL_INPUT_NCO1);
    smt_clear();

    smt_enable();
    smt_start();

    smt_interrupt_enable();

#ifdef DEVELOPMENT
    shell_register_command(sh_clockmon, "clockmon");
    shell_register_command(sh_uptime, "uptime");
#endif
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

/* ************************************************************************** */

//
#define CLOCK_CHECK_COOLDOWN 1000

int8_t clockmon_callback(char currentChar) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < CLOCK_CHECK_COOLDOWN) {
        return 0;
    }
    lastAttempt = get_current_time();

    printf("[%lu] \r\n", get_current_time());

    return 0;
}

// setup
void sh_clockmon(int argc, char **argv) {
    println("entering clockmon");

    shell_register_callback(clockmon_callback);
}

/* -------------------------------------------------------------------------- */

void sh_uptime(int argc, char **argv) {
    system_time_t currentTime = get_current_time();
    system_time_t milliseconds = currentTime;
    uint32_t seconds = milliseconds / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;

    milliseconds = milliseconds % 1000;
    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;

    printf(TXT_BLUE "[%lu]" TXT_RESET " uptime: %luD %luH %luM %luS %lumS",
           currentTime, days, hours, minutes, seconds, milliseconds);
    println("");
}