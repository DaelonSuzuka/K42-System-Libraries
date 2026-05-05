# OS Module Bug Register

Full audit of all .c and .h files completed 2026-05-05. Findings organized by severity.
All bugs need pair review before fixing — these are shared libraries, changes ripple.

## Critical

### tasks.c — check_task updates local copy, cooldown never persists
`task_t task = registry.taskList[taskID]` copies by value. `task.lastAttempt` update is lost.
Fix: write back `registry.taskList[taskID] = task` after update, or use pointer.
**Dead code** — no project uses this module. Abandoned in favor of the `attempt_*`
pattern (manual schedule). The header has no declarations. Safe to remove.

### shell_cursor.c — insert_char_at_cursor uint8_t wrap when cursor==0
Loop `for (uint8_t i = line->length; i >= line->cursor; i--)` — when cursor is 0, i wraps to 255
after decrementing past 0, causing infinite loop / buffer corruption.
Fix: use `int8_t` or restructure loop to avoid underflow.
**Live code** — triggers when inserting at position 0 in a non-empty line.

### judi.c — insert_character has no buffer bounds check
`buf->data[buf->length++] = currentChar` with no check against buffer size.
Long JSON messages overflow the buffer.
**Currently skipped in release builds** — only runs in dev with USB_ENABLED.

### judi.h — length field is uint8_t (max 255) but buffer is 256 bytes
`json_buffer_t.length` can't represent a completely full 256-byte buffer. Off-by-one.
Fix: use `uint16_t` or reduce `JSON_BUFFER_SIZE` to 255.

### records.c — get_record_address never returns NO_VALID_RECORD
Function always returns `records[recordID].start`, making the error check in `load_record`
dead code. `NO_VALID_RECORD` sentinel value is defined but never used.
**Not used in MC-200, but LIVE in AT-200ProIIv2** — used to store system flags
(SWR threshold, auto mode, peak mode, scale, HiLoZ). Error handling is broken
on shipped products.

## High

### stopwatch.c — ISR adds UINT16_MAX instead of 0x10000
Timer0 overflow counts 65536 ticks (0xFFFF→0x0000), but ISR adds 65535.
Systematic 1-tick error per overflow in microsecond stopwatch.
Fix: change to `+= 0x10000` or `+= 65536u`.

### logging.c — log_register__ iterates past valid entries
Loop goes to MAX_NUMBER_OF_FILES instead of numberOfFiles, calling strcmp with NULL entries.
Also no capacity check before writing to file[] array — buffer overflow possible.
**Live code** — triggers if more files register than MAX_NUMBER_OF_FILES allows.

### shell_command_processor.c — no bounds check on command registration
shell_register_command increments number_of_commands without checking against
MAXIMUM_NUM_OF_SHELL_COMMANDS. Buffer overflow if too many commands registered.
**Live code** — unlikely to hit in practice but no guard.

### shell_command_processor.h — wrong preprocessor guard
`#ifdef LOGGING_ENABLED` guards `shell_register_command` declaration, but should be
`#ifdef SHELL_ENABLED`. The command registration system has nothing to do with logging.
Currently harmless because both flags are typically defined together in dev builds.

### serial_port.c + usb_port.c — duplicate global uart symbol
Both use EMPTY_UART_INTERFACE(uart) at file scope. If both are linked, duplicate symbol.
Currently works because usb_port is conditionally compiled, but fragile.

## Medium

### shell_history.c — negative modulo on uint8_t head decrement
`(history.head - 1) % SHELL_HISTORY_LENGTH` — if head is 0, result is implementation-defined.
Fix: `(history.head + SHELL_HISTORY_LENGTH - 1) % SHELL_HISTORY_LENGTH`.

### shell.c — char signedness UB with iscntrl/isprint for values >= 128
shell_update(char currentChar) passes potentially signed char to ctype.h functions,
which is undefined behavior for negative values. Cast to unsigned char first.

### judi.c — message_timeout_error / message_stall_error are empty stubs
Error handlers have empty bodies. If a message times out or stalls, the buffer is never
reset, causing the protocol to hang indefinitely.

### message_builder.c — add_nodes has no termination guard
If input nodes array lacks the \e sentinel, the function reads past the end — buffer over-read.

### shell_builtins.c — &array with %s format specifier
`&productName` yields pointer-to-array, not pointer-to-char. Works in practice (same address)
but technically undefined behavior. Remove the & operator.

## Low / Code Smell

- judi.c: `find_key` returns 0 on failure — safe by construction (gperf perfect hash + enum layout: keywords at 5+, jsmn types at 0–4, token 0 hash=1 never matches), but ambiguous API
- judi.h: `judi_is_recieving` misspelled (should be receiving) — kept for API stability
- judi.c: `LOG_LEVEL` variable shadows the type name from logging.h
- libs/str_len.c: 70+ lines of commented-out dead code
- hash_function.c: returns -1 from potentially unsigned hash_value_t
- buttons.c: no bounds check on button index
- shell_keys.c: busy-wait in intercept_escape_sequence blocks system
- serial_port.c: `wtf` 2-byte buffer workaround for broken `uart.tx_char()` — every
  printf character becomes a null-terminated string print instead of a single byte
  write. Kills printf performance. `usb_port.c` uses `tx_char()` directly without
  the workaround. Only affects dev builds (shell uses printf, release doesn't).
  Cannot fix without hardware to verify.
- shell_utils.h: some macros use printf (stdout) instead of sh_print (serial port)

## Resolved

- **SKIP_JUDI_ENUMS guard** — judi.h duplicated `jsmntok_t`/`jsmntype_t` from jsmn.h, with a `#ifndef SKIP_JUDI_ENUMS` guard. Broke on newer XC8 (duplicate struct definitions). Fixed by splitting jsmn into `jsmn_types.h` (types only, no config) + `jsmn.h` (adds parser + functions). judi.h now includes `jsmn_types.h` directly; SKIP_JUDI_ENUMS deleted entirely.
