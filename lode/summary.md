# OS Module Summary

Operating system abstractions for bare-metal PIC18 Q41 projects. Provides cooperative multitasking primitives, shell, logging, and I/O utilities.

## Design Pattern

This is a **bare-metal superloop** architecture — no RTOS. All tasks must complete without blocking. Rate limiting uses `time_since()` for non-blocking delays.

## Directory Structure

```
src/os/
├── shell/              # Interactive command-line (dev builds only)
├── judi/               # JSON UART Device Interface protocol
├── json/               # JSMN JSON parser utilities
├── serial_port.c       # UART output abstraction
├── system_time.c       # Millisecond counter using NCO/SMT
├── buttons.c           # Debounced button input subsystem
└── ...
```

## Cooperative Superloop Pattern

```c
void main(void) {
    startup();
    while (1) {
        attempt_task_a();
        attempt_task_b();
        attempt_task_c();
        // ... more non-blocking tasks
    }
}

void attempt_my_task(void) {
    static system_time_t last_run = 0;
    if (time_since(last_run) < TASK_INTERVAL_MS) return;
    last_run = get_current_time();
    
    // Do work - must complete quickly!
}
```

Each `attempt_*` function checks if it's time to run and returns immediately if not.

## Dependency Injection

OS modules receive interfaces rather than owning hardware:

```c
// Shell needs UART - passed from project
void shell_init(uart_interface_t *uart);

// Buttons need pin readers - provided by project
void buttons_init(uint8_t count, button_check_t *readers);
```

## Key Principles

- **No blocking operations** — all tasks complete quickly
- **Dependency injection** — hardware abstractions passed from project
- **Dev/Release separation** — shell and JUDI excluded from release builds