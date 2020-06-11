#include "system_time.h"
#include "peripherals/timer.h"

/* ************************************************************************** */
// SMT helper macros
#define SMT_enable() SMT1CON0bits.EN = 1
#define SMT_disable() SMT1CON0bits.EN = 0

#define SMT_start() SMT1CON1bits.GO = 1
#define SMT_stop() SMT1CON1bits.GO = 0

#define SMT_clear() SMT1TMR = 0

#define SMT_interrupt_enable() PIE1bits.SMT1IE = 1
#define SMT_interrupt_disable() PIE1bits.SMT1IE = 0

#define SMT_mode(mode) SMT1CON1bits.MODE = mode
#define SMT_MODE_COUNTER 0b1000

#define SMT_signal(source) SMT1SIGbits.SSEL = source
#define SMT_SIGNAL_TMR2 0b00011

#define SMT_latch_now() SMT1STATbits.CPRUP = 1

void system_time_init(void) {
    // set timer2 to overflow in exactly 1mS
    timer2_clock_source(TMR_CLK_FOSC);
    timer2_prescale(TMR_PRE_1_128);
    timer2_postscale(TMR_POST_1_2);
    timer2_period_set(0xF9);

    timer2_start();

    // set Signal Measurement Timer to count the number of timer2 overflows
    SMT_mode(SMT_MODE_COUNTER);
    SMT_signal(SMT_SIGNAL_TMR2);
    SMT_clear();
    
    SMT_enable();
    SMT_start();

    SMT_interrupt_enable();
}

/* -------------------------------------------------------------------------- */

static volatile uint8_t smtOverflowCount;

void __interrupt(irq(SMT1), high_priority) SMT_overflow_ISR() {
    PIR1bits.SMT1IF = 0; // clear SMT interrupt flag

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
    SMT_interrupt_disable();
    SMT_latch_now();

    system_time_t currentTime = smtOverflowCount;
    currentTime <<= 24;
    currentTime += SMT1CPR;

    SMT_interrupt_enable();

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

#include "os/shell/shell_command_utils.h"

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

REGISTER_SHELL_COMMAND(sh_clockmon, "clockmon");