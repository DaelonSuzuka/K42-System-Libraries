# OS Module Quickstart

Load this file first when working with the OS submodule.

## What It Is

Bare-metal OS abstractions for PIC18. Cooperative superloop, shell, logging, buttons, system time, JUDI protocol.

## Key Rules

- **No blocking** — all `attempt_*` tasks must complete quickly
- **Dependency injection** — hardware interfaces passed from project, not owned
- **Dev/Release split** — shell and JUDI excluded from release via `#if DEVELOPMENT`

## Files We Touch Most

| File | What |
|------|-------|
| `shell/shell.c` | Shell engine, callbacks, TUI pattern |
| `shell/shell_builtins.c` | Built-in commands (uses system_information) |
| `buttons.c` | Timer4 ISR calls `scan_buttons()` every 5ms |
| `system_time.c` | NCO/SMT-based ms counter |
| `serial_port.c` | UART output abstraction |
| `logging.c` | Per-file log levels |

## Lode Index

- [summary.md](summary.md) — architecture overview
- [practices.md](practices.md) — coding patterns
- [superloop.md](superloop.md) — cooperative task scheduling
- [shell.md](shell.md) — shell and TUI subsystem
- [buttons.md](buttons.md) — debouncing and state
- [system-time.md](system-time.md) — timing infrastructure
- [startup.md](startup.md) — initialization sequence
- [layers.md](layers.md) — code organization
- [judi.md](judi.md) — JSON UART Device Interface
