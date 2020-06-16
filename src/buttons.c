#include "buttons.h"
#include "peripherals/timer.h"

/* ************************************************************************** */

static state_map_t states = {STATE_MASK, STATE_UP, STATE_DOWN, STATE_PRESSED,
                             STATE_RELEASED};

state_map_t new_state_map(void) {
    state_map_t newStateMap = {STATE_MASK, STATE_UP, STATE_DOWN, STATE_PRESSED,
                               STATE_RELEASED};
    return newStateMap;
}

void update_state_map(state_map_t newStateMap) { states = newStateMap; }

/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t history[MAX_NUMBER_OF_BUTTONS];
    uint8_t number;
    button_check_t functions[MAX_NUMBER_OF_BUTTONS];
} button_config_t;

static button_config_t buttons;

/* ************************************************************************** */

/*  These button checker functions have been derived from Eliot Williams
    'Ultimate Debouncer', found at Hackaday.

    The general idea is to use an interrupt routine that polls the physical
    buttons at a particular rate.  A history of button state is stored in a
    variable by bit shifting left one and setting the least-significant bit
    to the polled bit.

    These checker functions compare the button history against a bit mask to
    simultaneously debounce and determine one of 5 different states.
*/

/* ************************************************************************** */

void buttons_init(uint8_t numButtons, button_check_t *buttonFunctions) {
    // Initialize the button history array
    for (uint8_t i = 0; i < MAX_NUMBER_OF_BUTTONS; i++) {
        buttons.history[i] = 0;
    }

    buttons.number = numButtons;

    for (uint8_t i = 0; i < buttons.number; i++) {
        buttons.functions[i] = buttonFunctions[i];
    }
}

/* -------------------------------------------------------------------------- */

void scan_buttons(void) {
    for (uint8_t i = 0; i < buttons.number; i++) {
        buttons.history[i] <<= 1;
        buttons.history[i] |= buttons.functions[i]();
    }
}

/* ************************************************************************** */

// Returns 1 if any button is currently down
bool get_buttons(void) {
    for (uint8_t i = 0; i < buttons.number; i++) {
        if (btn_is_down(i))
            return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */

// Returns 1 if buttons entire history is 'down', aka button is being held.
bool btn_is_down(uint8_t button) {
    return (buttons.history[button] == states.down);
}

// Returns 1 if buttons entire history is 'up', aka button is NOT held.
bool btn_is_up(uint8_t button) {
    return (buttons.history[button] == states.up);
}

/*  Mask out the middle three 'bouncy' bits, and compare the result with the
    'pressed' binary literal.  This evaluates true if the button was
    previously 'up', bounces in the middle, and has three continuous polls
    of 'down'.

    If the above conditions are met, we set the history to all down, so
    another call to this function won't falsely detect another rising edge.
    Then, return 1 for true.

    If the conditions aren't met, return 0 for false. (Duh.)
*/
// Returns 1 if rising edge is detected
bool btn_is_pressed(uint8_t button) {
    if ((buttons.history[button] & states.mask) == states.pressed) {
        buttons.history[button] = states.down;
        return true;
    }
    return false;
}

/*  Mask out the middle three 'bouncy' bits, and compare the result with the
    'released' binary literal.  This evaluates true if the button was
    previously 'down', bounces in the middle, and has three continuous polls
    of 'up'.

    If the above conditions are met, we set the history to all up, so
    another call to this function won't falsely detect another falling edge.
    Then, return 1 for true.

    If the conditions aren't met, return 0 for false. (Duh.)
*/
// Returns 1 if falling edge is detected
bool btn_is_released(uint8_t button) {
    if ((buttons.history[button] & states.mask) == states.released) {
        buttons.history[button] = states.up;
        return true;
    }
    return false;
}
