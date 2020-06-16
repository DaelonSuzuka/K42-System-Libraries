#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

/*  Notes on using the button processing subsystem.

    This module is designed as a publisher/subscriber model, with two loosely
    coupled components.

    The publisher polls the physical buttons at a regular interval, 5ms is
    a typical value.  The publisher then records the current state of each
    button to a ledger or history.  This works best when it happens
    asynchronously with the subscriber half of the module, such as in an
    interrupt service routine triggered by a timer overflow, or a dedicated
    RTOS thread.

    The subscriber half is applied wherever it is desired to check on the status
    of an external input.  A call to one of the btn_is_xxxx() functions will
    look at the history that's been recorded by the publisher and return a yes
    or no, based on whether the history matches a predetermined pattern.  The
    specific pattern is different for each that is being evaluated, but they all
    provide some level of software debounce with hysteresis.
*/

/* ************************************************************************** */

// TODO: how does this work?

// default bit patterns assume that the buttons are active-low
#define STATE_MASK 0b11000111
#define STATE_UP 0b11111111
#define STATE_DOWN 0b00000000
#define STATE_PRESSED 0b11000000
#define STATE_RELEASED 0b00000111

typedef struct {
    uint8_t mask;
    uint8_t up;
    uint8_t down;
    uint8_t pressed;
    uint8_t released;
} state_map_t;

// Get an initialized state_map_t object
extern state_map_t new_state_map(void);

// Give the debouncer a new statemap
// If your buttons are active-low, then you don't need to use this
extern void update_state_map(state_map_t newStateMap);

/* -------------------------------------------------------------------------- */

// Storage for button history is statically allocated
#define MAX_NUMBER_OF_BUTTONS 16

// type for the button checking function pointer
typedef bool (*button_check_t)(void);

/* ************************************************************************** */

/*  buttons_init()

    numButtons: the number of buttons to register
    buttonFunctions: a pointer to an array of button checker functions
*/
extern void buttons_init(uint8_t numButtons, button_check_t *buttonFunctions);

/*  scan_buttons()

    This function calls each registered button_check_t function and updates the
    corresponding field in the stored button history.

    The application code is responsible for calling this function at whatever
    button polling rate you need. Any timing mechanism should be work: ISR, RTOS
    task, manual time_since() checking, it should all work.
*/
extern void scan_buttons(void);

/* ************************************************************************** */

/*  Notes on button states

    This button subsystem recognizes 4 distinct button states:
    pressed
    released
    down
    up

    The first two states represent the rising or falling edge of exactly when
    the user presses or released a button.

    The last two states represent when a button is either being held constantly
    or not being touched at all.
*/

// Returns true if any button is currently down
extern bool get_buttons(void);

// Returns true if the specified button has been down for 8 samples
extern bool btn_is_down(uint8_t button);

// Returns true if the specified button has been up for 8 samples
extern bool btn_is_up(uint8_t button);

// Returns true if the specified button is experiencing a falling edge
extern bool btn_is_pressed(uint8_t button);

// Returns true if the specified button is experiencing a rising edge
extern bool btn_is_released(uint8_t button);

#endif /* _BUTTONS_H_ */