/*
 * Host unit tests for button debounce (Eliot Williams-style history).
 *
 *   make test
 *
 * Uses scripted pin readers — no PIC, no mocks of hardware registers.
 * Default state map is active-low: pin 1 = up/released, pin 0 = down/pressed.
 */

#include "buttons.h"
#include <stdio.h>

static int failures = 0;

static void expect_true(const char *label, bool got) {
    if (!got) {
        printf("FAIL  %s (expected true)\n", label);
        failures++;
    } else {
        printf("ok    %s\n", label);
    }
}

static void expect_false(const char *label, bool got) {
    if (got) {
        printf("FAIL  %s (expected false)\n", label);
        failures++;
    } else {
        printf("ok    %s\n", label);
    }
}

/* -------------------------------------------------------------------------- */
/* Scripted pins */

static bool pin0_level = true; /* 1 = released (active-low idle) */
static bool pin1_level = true;

static bool read_pin0(void) { return pin0_level; }
static bool read_pin1(void) { return pin1_level; }

static button_check_t readers1[1] = {read_pin0};
static button_check_t readers2[2] = {read_pin0, read_pin1};

static void scan_n(unsigned n) {
    while (n--)
        scan_buttons();
}

static void settle_up0(void) {
    pin0_level = true;
    scan_n(8);
}

static void settle_down0(void) {
    pin0_level = false;
    scan_n(8);
}

/* -------------------------------------------------------------------------- */

static void test_idle_up_after_polls(void) {
    printf("=== idle / up ===\n");
    buttons_init(1, readers1);
    /* Fresh history is 0x00 (looks "down"); need polls of released pin */
    expect_false("fresh: not up yet", btn_is_up(0));
    settle_up0();
    expect_true("after 8 high polls: up", btn_is_up(0));
    expect_false("after 8 high polls: not down", btn_is_down(0));
    expect_false("after 8 high polls: not pressed edge", btn_is_pressed(0));
    expect_false("get_buttons false when up", get_buttons());
}

static void test_press_edge_once(void) {
    printf("\n=== press edge ===\n");
    buttons_init(1, readers1);
    settle_up0();

    /* From 0xFF, three low samples → history matches STATE_PRESSED under mask */
    pin0_level = false;
    scan_n(3);
    expect_true("press edge on clean down", btn_is_pressed(0));
    /* btn_is_pressed latches history to all-down so the edge is not sticky */
    expect_false("press edge not sticky", btn_is_pressed(0));
    expect_true("held: down after press latch", btn_is_down(0));
    expect_true("get_buttons true when down", get_buttons());
}

static void test_release_edge_once(void) {
    printf("\n=== release edge ===\n");
    buttons_init(1, readers1);
    settle_down0();
    expect_true("held: down", btn_is_down(0));

    pin0_level = true;
    scan_n(3);
    expect_true("release edge on clean up", btn_is_released(0));
    expect_false("release edge not sticky", btn_is_released(0));
    expect_true("idle: up after release latch", btn_is_up(0));
}

static void test_no_false_press_on_single_dip(void) {
    printf("\n=== no false press ===\n");
    buttons_init(1, readers1);
    settle_up0();
    /* Single low sample — not enough for three continuous downs at LSB */
    pin0_level = false;
    scan_buttons();
    pin0_level = true;
    scan_n(2);
    expect_false("single dip does not press", btn_is_pressed(0));
    settle_up0();
    expect_true("back to up", btn_is_up(0));
}

static void test_masked_bounce_still_presses(void) {
    printf("\n=== masked bounce still presses ===\n");
    buttons_init(1, readers1);
    settle_up0();

    /*
     * STATE_MASK 0b11000111 ignores bits 5..3. From 0xFF, samples
     * 0,1,0,0,0 → history 0xE8 (0b11101000): bounce only in ignored bits,
     * outer bits still match STATE_PRESSED.
     */
    pin0_level = false;
    scan_buttons();
    pin0_level = true;
    scan_buttons();
    pin0_level = false;
    scan_n(3);
    expect_true("press despite bounce in mask hole", btn_is_pressed(0));
}

static void test_two_buttons_independent(void) {
    printf("\n=== two buttons ===\n");
    buttons_init(2, readers2);
    pin0_level = pin1_level = true;
    scan_n(8);
    expect_true("btn0 up", btn_is_up(0));
    expect_true("btn1 up", btn_is_up(1));

    pin0_level = false;
    scan_n(3);
    expect_true("btn0 pressed", btn_is_pressed(0));
    expect_false("btn1 not pressed", btn_is_pressed(1));
    expect_true("btn1 still up", btn_is_up(1));
}

int main(void) {
    test_idle_up_after_polls();
    test_press_edge_once();
    test_release_edge_once();
    test_no_false_press_on_single_dip();
    test_masked_bounce_still_presses();
    test_two_buttons_independent();

    if (failures) {
        printf("\n%d FAILURE(S)\n", failures);
        return 1;
    }
    printf("\nall passed\n");
    return 0;
}
