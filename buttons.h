#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include "hardware.h"
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
/*  Notes on BUTTON_LIST:

    BUTTON_LIST is defined using X Macros. From Wikipedia: "X Macros are a
    technique for reliable maintenance of parallel lists, of code or data, whose
    corresponding items must appear in the same order. They are most useful
    where at least some of the lists cannot be composed by indexing, such as
    compile time." (see: https://en.wikipedia.org/wiki/X_Macro)

    Here is a generic, example BUTTON_LIST macro that can be used as a template:

    #define BUTTON_LIST \
    X(FORWARD)\
    X(BACK)\
    X(UP)\
    X(DOWN)

    The project software MUST define the following macros:

    NUMBER_OF_BUTTONS - macro containing a non-negative, non-zero integer
    BUTTON_LIST - X macro containing a list of button names
    BUTTON_PIN_NAME_FORMAT - macro that converts NAME to matching pin name

    Each button name "NAME" defined in BUTTON_LIST must have a matching
    "NAME_BUTTON_PIN" macro defined in pins.h.
*/

// Make sure the required macros are defined in the project
#ifndef NUMBER_OF_BUTTONS
    #error "NUMBER_OF_BUTTONS is not defined"
#elif NUMBER_OF_BUTTONS > 32
    #error "Can't have more than 32 buttons!"
#endif

#if NUMBER_OF_BUTTONS > 0
    #ifndef BUTTON_LIST
        #error "BUTTON_LIST is not defined"
    #endif

    #ifndef BUTTON_PIN_NAME_FORMAT
        #error "BUTTON_PIN_NAME_FORMAT is not defined"
    #endif
#else
    #undef BUTTON_LIST 
    #define BUTTON_LIST BLANK_LIST 
#endif

// X Macro, expands BUTTON_LiST into contents of buttonName_t enum
#define X(name) name,
typedef enum { BUTTON_LIST } buttonName_t;
#undef X

/* ************************************************************************** */

// Setup
extern void buttons_init(void);

// Returns true if any button is currently down
extern bool get_buttons(void);

/* -------------------------------------------------------------------------- */

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

// Returns true if the specified button has been down for 8 samples
extern bool btn_is_down(buttonName_t buttonName);

// Returns true if the specified button has been up for 8 samples
extern bool btn_is_up(buttonName_t buttonName);

// Returns true if the specified button is experiencing a falling edge
extern bool btn_is_pressed(buttonName_t buttonName);

// Returns true if the specified button is experiencing a rising edge
extern bool btn_is_released(buttonName_t buttonName);

#endif /* _BUTTONS_H_ */