# OS Module Quickstart

Load this file first when working with the OS submodule. **Also load `practices.md` at session start regardless of which project you're in** — it contains the design philosophy (semantic compression, billboards, correctness principles) that governs all code in this suite.

## What It Is

Bare-metal OS abstractions for PIC18. Cooperative superloop, shell, logging, buttons, system time, JUDI protocol.

## Key Rules

- **No blocking** — all `attempt_*` tasks must complete quickly
- **Dependency injection** — hardware interfaces passed from project, not owned
- **Dev/Release split** — shell and JUDI excluded from release via `#if DEVELOPMENT`
- **Codegen composition** — OS modules consume generated code (pin helpers, button arrays,
  PPS macros) but don't own the codegen. The project's `pinmap.py` is the single source
  of truth; see [toolchain codegen docs](../../toolchain/lode/codegen.md) and
  https://daelon.dev/posts/codegen1/

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
- [practices.md](practices.md) — coding patterns **AND** design philosophy (load at every session start)
- [terminology.md](terminology.md) — domain terms + pointers to philosophy definitions
- [superloop.md](superloop.md) — cooperative task scheduling
- [shell.md](shell.md) — shell and TUI subsystem
- [buttons.md](buttons.md) — debouncing and state
- [system-time.md](system-time.md) — timing infrastructure
- [startup.md](startup.md) — initialization sequence
- [layers.md](layers.md) — code organization
- [judi.md](judi.md) — JSON UART Device Interface

## Setting Up a New Project Lode

When creating or onboarding a new project, follow these instructions to set up its project-level lode. The OS lode (this file) is the boot sector — it's already populated after `git submodule update --init`.

### 1. Create the lode/ directory

```
lode/
    summary.md          # One-paragraph living snapshot of this project
    terminology.md      # Project-specific terms + OS philosophy pointers (see below)
    practices.md        # Project-specific practices ONLY (reference OS practices for shared patterns)
    roadmap.md          # Vetted active work items
    todo.md             # Raw one-line ideas
    plans/              # Detailed implementation plans
    tmp/                # git-ignored session scraps
```

### 2. Populate terminology.md

Include project-specific terms. Then add the OS philosophy pointers that every project needs — copy these entries verbatim:

```
- **Billboard** → os/lode/practices.md — one instance, one writer, many readers, always current
- **Semantic Compression** → os/lode/practices.md — write inline, compress on second instance
- **Events Layer** → os/lode/practices.md — shared processing between UI stimuli and logic
- **Correctness by Boundaries** → os/lode/practices.md — stimulus→processing→response
- **Correctness by Construction** → os/lode/practices.md — structure makes wrong states impossible
- **Don't fuck with it** → os/lode/practices.md — working code stays working
- **Fail loudly** → os/lode/practices.md — bugs should be catastrophic, not subtle
```

### 3. Populate practices.md

Only project-specific items. Reference OS practices for everything else. Typical contents:
- Pin configuration table
- Main loop rates
- Debug UART settings
- Flash/RAM constraints for this chip
- Submodule pointers and rules (reference OS practices for the origin story)
- Chip-specific gaps (e.g. Q41 vs K42)
- Build commands (duplicate — direct dependency)

### 4. Session startup protocol

For ANY project in this suite, at session start:
1. Read the project's `lode/summary.md`, `lode/terminology.md`, `lode/roadmap.md`
2. Read `src/os/lode/quickstart.md` (this file)
3. Read `src/os/lode/practices.md` — non-negotiable, contains the design philosophy
4. Read the project's `lode/practices.md` for project-specific details
