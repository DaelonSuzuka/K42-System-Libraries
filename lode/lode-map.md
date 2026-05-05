# Lode Map - OS Module

Index of documentation for the OS module.

## Entry Point

- [quickstart.md](quickstart.md) - **Load first.** What this module is, key rules, files we touch most, project lode setup instructions, session startup protocol

## Core Files

- [summary.md](summary.md) - Module overview and cooperative superloop pattern
- [terminology.md](terminology.md) - Domain language + philosophy term pointers (billboard, semantic compression, correctness principles)
- [practices.md](practices.md) - **PLATFORM HOME.** Design philosophy, coding conventions, origin story, firmware provenance, datasheet cautionary tale

## Architecture Patterns

- [superloop.md](superloop.md) - The `attempt_*()` pattern, stack-as-state, three-phase hold handlers, submenu recursion
- [startup.md](startup.md) - Layered initialization sequence
- [layers.md](layers.md) - Three-layer architecture (peripherals/OS/application), pragmatic boundary
- [autostart.md](autostart.md) - Linker-section self-registration pattern (not adopted, tradeoff analysis)

## Subsystems

- [shell.md](shell.md) - Chitin v0.6.0: three modes, escape decoder, TUI callbacks, blocking loop issue
- [judi.md](judi.md) - JUDI architecture: jsmn split, gperf hash codegen, node system, protocol gaps, memory cost
- [buttons.md](buttons.md) - Ultimate Debouncer, codegen composition, publisher/subscriber
- [system-time.md](system-time.md) - NCO→SMT timing, 32-bit composition, timing chain of trust, known issues
- [logging.md](logging.md) - Block-consuming macros, logedit origin, bitfield aspirational design

## Audit & Bug Tracking

- [bug-register.md](bug-register.md) - Full audit findings organized by severity (pair review before fixing)

## Undocumented Source Files

- `serial_port.c` - UART output abstraction
- `stopwatch.c` - Stopwatch utilities (has known UINT16_MAX bug)
- `tasks.c` - Dead code, never called, safe to remove
- `records.c` - NVM-backed settings storage (live in AT-200Pro, unused in MC-200)

---

*This file is maintained by the AI. Update when adding new documentation.*