# Logging Module

Conditional code blocks with zero-cost elimination and runtime level control.

## The Core Idea

`LOG_*` and `ON_*` macros consume an arbitrary **block of code**, not a format string:

```c
LOG_DEBUG({
    printf("channel: %d, ", channel);
    printf("raw: % 4u, ", measurement);
    printf("adjusted: % 4u", convert_to_millivolts(measurement, maxVoltage[channel]));
    println("");
});
```

When `LOGGING_ENABLED` is not defined, the macro compiles to nothing — the entire
block is syntactically eliminated. The compiler never sees it. Zero cost in release.

This is not a logging function. It's a **code injection point** gated by log level.

## Why Blocks, Not Format Strings

Most logging frameworks give you `LOG_DEBUG(fmt, ...)`. This gives you `LOG_DEBUG({ anything })`.

- **No format string restrictions** — multiple printfs, helper calls, conditionals, loops
- **Works with custom data structures** — write `print_thing()` once, call it from shell commands AND log blocks
- **Not limited to printing** — the block can do anything: prepare data, modify behavior, collect metrics

The `ON_*` / `LOG_*` split:
- `ON_DEBUG({ ... })` — conditionally execute the block (no header)
- `LOG_DEBUG({ ... })` — print header, then conditionally execute the block

`ON_*` enables behavior-gating: `ON_DEBUG({ enable_extra_checks = true; })` makes the
system more thorough when being inspected. The code exists in the binary but only
executes when the level is raised. Most logging frameworks can only gate output;
this can gate *execution*.

## How It Works

Each .c file that uses logging declares a file-local `LOG_LEVEL`:

```c
static uint8_t LOG_LEVEL = L_SILENT;
```

`log_register()` (a macro) passes `__FILE__` and `&LOG_LEVEL` to the logging database.
This gives `logedit` a pointer to mutate the file-local static at runtime — no getter/setter needed.

The `ON_*` / `LOG_*` macros compare the current level:

```c
// Execute block if level is high enough (no header)
ON_DEBUG({ do_thing(); })

// Print header then execute block
LOG_DEBUG({ printf("value: %d\r\n", x); })
```

When `LOGGING_ENABLED` is not defined, all macros compile to nothing — zero cost in release.

## Level Hierarchy (ordinal)

```
L_SILENT < L_FATAL < L_ERROR < L_WARN < L_INFO < L_DEBUG < L_TRACE
```

Setting level to `L_WARN` enables WARN, ERROR, and FATAL output. This is the current model.
The bitfield design (commented out in logging.h) would allow independent level toggling.

## logedit — Interactive Level Editor

`logedit` is a full-screen TUI that runs inside the shell session. It was the first TUI
program and the reason the shell's callback/input-redirection architecture exists.

On entry, `logedit` sets ALL log levels to SILENT. This is mandatory — unsuppressed
log output would corrupt the logedit screen. On Enter, new levels are written back.
On Escape, original levels are restored.

**Risk:** If the session is lost (USB disconnect, crash), levels remain SILENT with
no recovery path short of power-cycle.

## API Surface

| Function / Macro | Purpose |
|---|---|
| `log_register()` | Register current file with log database |
| `ON_<LEVEL>(block)` | Conditionally execute block (no header) |
| `LOG_<LEVEL>(block)` | Print header + conditionally execute block |
| `log_level_edit(id, level)` | Set level by file ID |
| `set_log_level(name, level)` | Set level by filename |
| `print_log_header(level, file, line)` | Format and print a log message header |

## Known Bugs

See [bug-register.md](bug-register.md) — logging section:
- `log_register__` iterates past valid entries, calling strcmp with NULL
- No capacity check before writing to file[] array
- `look_up_file_id` returns 0 on failure (ambiguous with valid ID 0)
- `push/pop_log_level` — dead code, single-slot impl can't nest
- Magic number 6 for level ceiling in logedit_keys

## Dead Code / Unused API

- `ON_*` macros — never used in any project file. Available but untested in practice.
- `push_log_level` / `pop_log_level` — declared, implemented, never called.
  Intended for "raise level around a section, put it back after" debugging.
  Broken by single `savedLevel` variable — can't nest or push for multiple files.

## Aspirational: Bitfield Levels

Commented-out `log_settings_t` in logging.h proposes bit-per-level instead of ordinal:
each level is an independent toggle rather than a cumulative threshold. Would change
`ON_*` / `LOG_*` from ordinal comparison to bitmask test. UI change in logedit is
minor — grid of toggleable cells instead of ordinal selector. Data model is ready
if someone wants to implement it. Anonymous union/struct is C11 though.
