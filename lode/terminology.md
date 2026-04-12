# Terminology

Domain-specific terms used in the OS module.

## Architecture Terms

- **Superloop** - Bare-metal main loop with cooperative multitasking; no RTOS
- **attempt_* pattern** - Non-blocking task functions that check timing and return immediately
- **time_since()** - Pattern for non-blocking delays using millisecond counter

## Shell Terms

- **Shell Command** - Function registered with the shell, called with argc/argv
- **Callback Mode** - Interactive shell mode where keypresses are handled one at a time
- **Builtins** - Built-in shell commands (help, clear, etc.)

## JUDI Terms

- **JUDI** - JSON UART Device Interface; JSON-based protocol for serial communication
- **Token** - Parsed JSON element from jsmn parser
- **Hash** - Integer hash of string key for fast lookup

## Button Terms

- **Publisher** - ISR or timer that scans button pins and updates history
- **Subscriber** - Application code that queries debounced button state
- **History** - 8-bit sliding window of past button readings

## Timing Terms

- **NCO** - Numerically Controlled Oscillator; generates precise pulse trains
- **SMT** - Signal Measurement Timer; 24-bit counter peripheral
- **system_time_t** - uint32_t milliseconds since boot