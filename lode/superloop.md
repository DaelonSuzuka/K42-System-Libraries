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