# Cooperative Superloop Pattern

Bare-metal task scheduling without an RTOS. All task functions must complete without blocking.

## The Pattern

Every periodic task follows this exact pattern:

```c
#define TASK_RATE_HZ 30
#define TASK_COOLDOWN (1000 / TASK_RATE_HZ)

bool attempt_task(void) {
    static system_time_t lastAttempt = 0;
    
    if (time_since(lastAttempt) < TASK_COOLDOWN) {
        return false;  // Not time yet, skip
    }
    
    lastAttempt = get_current_time();
    
    // Do the work - MUST complete quickly
    do_task_work();
    
    return true;  // Work was done this call
}
```

### Key Elements

1. **`static` timing variable** - Each function owns its `lastAttempt` timer
2. **Rate constant** - Define rate in Hz, calculate cooldown in ms
3. **`time_since()` check** - Non-blocking; returns immediately if not time
4. **Return bool** - `true` = work done, `false` = skipped

## The Main Loop

```c
void main(void) {
    startup();
    while (1) {
        attempt_task_a();    // 50 Hz
        attempt_task_b();    // 30 Hz
        attempt_task_c();    // 10 Hz
        // ... more non-blocking tasks
    }
}
```

Each `attempt_*` function checks its own timing and returns immediately if not time to run.

## Why This Works

| Property | Benefit |
|----------|---------|
| State encapsulation | Each function owns its timer via `static` |
| Non-blocking | Returns immediately if not time to run |
| Self-throttling | Rate limit built in, no external scheduler |
| Boolean feedback | Returns `true` if work done, `false` if skipped |

## Rate Limiting Example

| Task | Rate | Cooldown (ms) |
|------|------|---------------|
| Sensor reading | 50 Hz | 20 |
| Display update | 30 Hz | 33 |
| Button handling | 30 Hz | 33 |
| Serial poll | Every loop | 0 (always check) |

## The `time_since()` Helper

```c
// Handles overflow correctly due to unsigned arithmetic
system_time_t time_since(system_time_t startTime) {
    return get_current_time() - startTime;
}
```

Even when `current_time < startTime` (overflow happened), unsigned subtraction yields correct elapsed time.

## Anti-Patterns

```c
// ❌ WRONG - Blocks the superloop
void bad_task(void) {
    while (!ready) {
        // Waiting blocks everything!
    }
}

// ❌ WRONG - No rate limiting
void bad_task(void) {
    do_work();  // Runs every loop iteration!
}

// ❌ WRONG - Using delay
void bad_task(void) {
    delay_ms(100);  // Blocks for 100ms!
    do_work();
}

// ✅ CORRECT - Non-blocking, rate-limited
bool attempt_task(void) {
    static system_time_t last = 0;
    if (time_since(last) < COOLDOWN) return false;
    last = get_current_time();
    do_work();
    return true;
}
```

## Rules for Adding New Tasks

1. **Create `attempt_<task>()`** - Follow the pattern exactly
2. **Use `static` variables** - For timing and any task state
3. **Never block** - No `while` waiting, no `delay_ms()`
4. **Return quickly** - Complete in milliseconds
5. **Return bool** - `true` = work done, `false` = skipped

## Why No RTOS?

For small PIC18 devices:
- Limited RAM (~4KB typical)
- No hardware threading support
- Cooperative superloop uses minimal stack (single call stack)
- Deterministic timing (no context switch overhead)
- Simple to debug (single execution path)

The system has tasks but no scheduler — it has a **manual schedule**. The
programmer arranges call order in the main loop and each task checks its own
timing. The programmer IS the scheduler. An earlier attempt at a centralized
task scheduler (`tasks.c`) was abandoned in favor of the `attempt_*` pattern —
the inside-out version where each task owns its timing rather than being
called by an external scheduler.

## The Stack-as-State Pattern

For UI-driven devices with button holds (tuners, meters with front panels), the
superloop extends into a distributed state machine where **the state is the call
stack, not a variable.** Each "state" is a function on the stack:

```
ui_mainloop()          ← idle state
  └─ tune_hold()       ← tune-button-held state
       └─ ui_idle_block()  ← background tasks still run
```

Hold functions (power_hold, tune_hold, relay_button_hold) are "blocking" loops
that call `ui_idle_block()` on every iteration. The system never stops servicing
background tasks — the idle work happens *inside* the busy function.

This inverts the typical approach: instead of making blocking functions yield
to a scheduler, the blocking functions *call the scheduler*. No state enum, no
switch/case dispatch, no state persistence. Local variables on the stack ARE
the state. The function returns when the state is over.

See AT-200ProIIv2 for the full pattern:
- `ui/ui_power_button.c` — simple hold with timeout
- `ui/ui_tune_button.c` — time-pressured hold with animation
- `ui/ui_relay_button.c` — complex hold with retrigger, animation, "dragons"

### Three-Phase Hold Handler

Every hold handler implicitly provides three places to put code, controlling
*when* a physical response happens:

```c
void button_hold(void) {
    // PHASE 1: On press — runs once, immediately
    system_time_t startTime = get_current_time();

    while (btn_is_down(BUTTON)) {
        // PHASE 2: During hold — runs every iteration
        // Loop automatically exits when button is released
        show_feedback(time_since(startTime));
        ui_idle_block();
    }

    // PHASE 3: After release — runs once, before returning
    dispatch_action_based_on_hold_duration(time_since(startTime));
}
```

No phase-tracking variable. No state enum. The three phases are just the
structure of a function with a while loop. Physical interaction design maps
directly to code structure: "respond immediately" → phase 1, "respond while
held" → phase 2, "respond on release" → phase 3.

### Submenu Recursion

The stack-as-state pattern composes recursively. Entering a submenu pushes
another function onto the call stack:

```
ui_mainloop → func_hold → function_submenu → scale_submenu
```

Each level has its own `ui_idle_block()` call. The state unwinds by returning.

### RF Escape Hatch

Nearly every hold handler and submenu checks `RF_is_present()` and immediately
returns or breaks. Radio takes priority over UI. A `return` from inside a hold
handler pops the entire stack back to `ui_mainloop`. No cleanup needed — the
stack unwinding IS the cleanup.

### Display Mutex

Hold functions call `disable_bargraph_updates()` on entry and
`enable_bargraph_updates()` on exit. This prevents `ui_idle_block()` from
overwriting the display while a hold handler is showing its own content.

### The One `delay_ms()` in Production Code

`function_submenu()` uses `delay_ms(50)` as a debounce gap between FUNC release
and submenu activation. This is the only known use of `delay_ms()` outside
init code. 50ms of blocking is acceptable here — the user just released a
button, nothing time-critical is happening. This might be the edge case that
would benefit from an idle-injectable delay.

## Reentrancy Warning

PIC18 only partially supports reentrancy:

- Don't call a task function from within itself
- Don't call a task function from an ISR that might interrupt it
- If an ISR shares data with main loop, use critical sections:

```c
void attempt_shared_task(void) {
    static system_time_t last = 0;
    if (time_since(last) < COOLDOWN) return false;
    last = get_current_time();
    
    begin_critical_section();
    // Access shared data
    end_critical_section();
    
    return true;
}
```

## Related Files

| File | Purpose |
|------|---------|
| `system_time.c` | Millisecond counter and `time_since()` |
| `system.c` | Startup orchestration |
| Main application | Calls `attempt_*` functions |