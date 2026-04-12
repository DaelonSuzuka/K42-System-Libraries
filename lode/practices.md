# Coding Practices

Patterns and conventions for bare-metal PIC18 development.

## Initialization Pattern

All modules follow a consistent initialization pattern:

```c
// In module header
void module_init(void);

// In module source
void module_init(void) {
    // 1. Configure peripheral registers
    // 2. Set up PPS if needed
    // 3. Initialize state variables
    // 4. Register with OS services (shell commands, etc.)
}
```

The startup sequence:

```c
void startup(void) {
    system_init();      // Oscillator, interrupts, ports
    os_init();           // Shell, buttons, logging, timers
    application_init();  // Product-specific modules
    pps_lock();          // Lock PPS before main loop
}
```

## UART Dependency Injection

UARTs use an interface pattern:

```c
// Project-level configuration
uart_config_t config = UART_get_config(2);
config.baud = _1000000;
config.txPin = PPS_DEBUG_TX_PIN;
config.rxPin = PPS_DEBUG_RX_PIN;
create_uart_buffers(debug, config, 255);

uart_interface_t uart = UART_init(&config);

// Pass to module that needs it
some_module_init(&uart);
```

## Conditional Compilation

Development features gated by preprocessor:

```c
#ifdef DEVELOPMENT
    shell_register_command(sh_debug, "debug");
#endif

#ifdef SHELL_ENABLED
    shell_init();
#endif

#ifdef LOGGING_ENABLED
    logging_init();
#endif
```

Release builds exclude debug code entirely via `skip_rules` in `project.yaml`:

```yaml
release:
  skip_rules:
    - src/shellcommands/*
    - src/os/shell/*
```

## Shell Commands

```c
void sh_mycommand(int argc, char **argv) {
    switch (argc) {
    case 1:
        print_usage();
        return;
    case 2:
        if (!strcmp(argv[1], "list")) {
            do_list();
            return;
        }
        break;
    }
    println("invalid arguments");
}

// Register in init function
#ifdef SHELL_ENABLED
    shell_register_command(sh_mycommand, "mycommand");
#endif
```

## Pin Definitions

Never edit `pins.h` or `pins.c` directly. Edit `pinmap.py`:

```python
# pinmap.py
common = {
    'A2': ('BRIGHTNESS_PIN', Pin.analog_in),
    'B5': ('DEBUG_RX_PIN', Pin.uart_rx),
    'B7': ('DEBUG_TX_PIN', Pin.uart_tx),
}
```

Run `make compile` to regenerate.

## Code Generation

Use cog for lookup tables and constants:

```c
#define TABLE_SIZE 32

/* [[[cog
    TABLE_SIZE = 32  # Must match C #define
    
    cog.outl('static const uint8_t table[TABLE_SIZE] = {')
    for i in range(TABLE_SIZE):
        val = compute_lookup_value(i)
        cog.outl(f'    {val},')
    cog.outl('};')
]]] */
// Generated code here
// [[[end]]]
```

## Critical Sections

Use when ISRs share data with main loop:

```c
#include "peripherals/interrupt.h"

void attempt_shared_task(void) {
    // Check timing
    if (time_since(last) < COOLDOWN) return false;
    last = get_current_time();
    
    // Access shared data
    begin_critical_section();
    shared_value = isr_updated_value;
    end_critical_section();
    
    return true;
}
```

## Rate Limiting

Every periodic task uses `time_since()`:

```c
#define MY_RATE_HZ 30
#define MY_COOLDOWN (1000 / MY_RATE_HZ)

bool attempt_my_task(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < MY_COOLDOWN) return false;
    lastAttempt = get_current_time();
    
    // Do work here
    return true;
}
```

## PPS Configuration

Each module owns its PPS settings:

```c
void my_peripheral_init(void) {
    // Configure PPS for this peripheral
    pps_in_UART1_RX(PPS_MY_RX_PIN);
    pps_out_UART1_TX(PPS_MY_TX_PIN);
    
    // Then initialize peripheral
    UART_init(&config);
}
```

PPS is locked once after all modules initialized:

```c
void startup(void) {
    // ... all _init calls ...
    pps_lock();  // No more PPS changes
}
```