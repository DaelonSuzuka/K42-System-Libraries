# Terminology

Domain-specific terms used in the OS module. Terms marked with → are defined in detail in [practices.md](practices.md).

## Architecture Terms

- **Superloop** - Bare-metal main loop with cooperative multitasking; no RTOS
- **attempt_* pattern** - Non-blocking task functions that check timing and return immediately
- **time_since()** - Pattern for non-blocking delays using millisecond counter
- **Billboard** → - A global that's okay: one instance, one writer, many readers, always current (e.g. currentRF, systemFlags)
- **Semantic Compression** → - Write inline, compress on second instance, never preemptively abstract
- **Events Layer** → - Shared processing between UI stimuli (buttons, shell, JUDI) and business logic

## Correctness Terms

- **Correctness by Boundaries** → - stimulus→processing→response keeps state contained
- **Correctness by Construction** → - the structure makes wrong states impossible
- **Don't fuck with it** → - working code isn't refactored for aesthetics; stability is a feature
- **Fail loudly** → - if something's wrong, it should obviously break, not be subtly wrong

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