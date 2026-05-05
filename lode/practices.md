# Coding Practices

Patterns and conventions for bare-metal PIC18 development. This is the platform home — every project includes the OS, so shared philosophy and patterns live here.

## Design Philosophy

### Stimulus → Processing → Response

Every module is structured around clean stimulus → processing → response boundaries. This isn't just organization — it's what makes the codebase stable without automated tests. When those boundaries are clear, modules become naturally testable through the shell (each `sh_` command is a manual test harness), and bugs have few places to hide because state doesn't leak across boundaries.

### Correctness by Boundaries, Construction, and Restraint

- **Correctness by boundaries** — stimulus→processing→response keeps state contained and interactions explicit
- **Correctness by construction** — the structure makes the wrong state impossible (gperf perfect hash can't collide, enum layout prevents ambiguous returns, call stack is the exclusion mechanism)
- **Once it's working, don't fuck with it** — the OS libraries have been stable for years because working code isn't refactored for aesthetics. Stability is a feature.

The goal: nothing should be *subtly* wrong. If something is wrong, it should obviously break — not silently produce slightly incorrect results that compound over time. (Spolsky influence.)

### Semantic Compression

Development method per [Casey Muratori](https://caseymuratori.com/blog_0015): write code inline first. Write it again when needed. On the second instance, compress the common part into a reusable form. Never preemptively abstract — "make it usable before you make it reusable."

Objects are constructs that emerge from procedural reuse, not things you design first. Spend zero time thinking about "what goes where" — write the procedure, and the structure will reveal itself. The method was discovered through practice before it was articulated through theory; the PIC18 constraints taught compression, the article provided the vocabulary.

### Billboards

A **billboard** is a global variable that's okay. Criteria: exactly one instance, one writer, many readers, always representing the current state. `currentRF`, `systemFlags`, `currentRelays`, `shell` — all billboards. Everyone needs the same data, it changes continuously, and by the time you wrapped it in a getter the data would be stale anyway. On the compression principle, you'd only abstract access behind an interface if you had two implementations. One instance = no compression opportunity = no abstraction.

### Events Layer

`events.c` (per project) is the reusable processing layer between UI stimuli (buttons, shell, JUDI) and business logic (relays, flags, tuning). It arose from compression — when the shell needed to do the same things as the hold handlers, two stimuli producing the same response meant the processing was pulled into a shared layer. Not designed for reuse; compressed into reuse from two instances. Testability is a *consequence* of the boundary, not a separate goal.

### Testing Philosophy

Code is written to be testable (functional, clear boundaries) but not tested automatically (no test framework, no host compilation path). The shell is the test infrastructure — `rfmon` tests the RF sensor, `logedit` tests logging, `backlightmon` tests the backlight. Every interactive command exists partly to exercise its module. At this codebase scale, with one author who knows every call site, that's a pragmatic balance.

### Read the Fucking Datasheet

The datasheet always has a better way. The NCO→SMT trick, the SMT capture register for atomic reads, the PPS system — all found by reading the PIC datasheet cover to cover. The most expensive version of this lesson: a contractor swapped PIC18F4520→45K22 (pin-compatible, 50mA less GPIO source current) without reading the electrical specs. Six relay coils switching simultaneously under RF load exceeded the new chip's current budget → VDD brownout → relay actuator flutter → contact arcing → welded relays. Pin-compatible doesn't mean electrically identical.

## Concrete Patterns

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

## Origin Story

LDG lost the ability to compile its own firmware due to poor contractor management. The contractor used Perforce but LDG never had repo access — source was delivered as zip files of the project folder via email. Version control was a folder called "old" inside the project folder: drag current contents into "old", unzip the new zip. This produced chains of nested "old" folders stretching back years, none of which could be positively correlated to a shipped firmware version. The code itself was monolithic — entire products in single .c files, no layers, no codegen, blocking `pause_ms()` in LED animations, 16 copy-paste product variants with hand-tweaked calibration constants.

Every design decision in the current architecture is a reaction to that world. The submodules with pinned commits, the git hash baked into every hex, the layered code with clear boundaries, the codegen pipeline — all exist so that never happens again. A core goal: clone fresh, build with zero fuss, any time, any machine.

## Firmware Provenance

The git commit hash is baked into every hex file as `__PRODUCT_VERSION__` (set in `toolchain/scripts/xc8_cc.py`). The shell `ver` command prints it. Any physical device can be traced back to its exact source code. The direct opposite of the "old folder" version control that preceded it.