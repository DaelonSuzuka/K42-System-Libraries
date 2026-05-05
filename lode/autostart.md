# Autostart Pattern

Linker-section-based self-registration for init functions.

## The Pattern

Each module self-registers by placing a function pointer into a named linker section:

```c
// auto_start.h
typedef struct {
    void (*function)(void);
} auto_start_t;

#define AUTO_START(function) \
const auto_start_t __section("auto_start") function##_ptr = {function}

// auto_start.c
extern const char _Lauto_start[];  // linker-defined section start
extern const char _Hauto_start[];  // linker-defined section end

const auto_start_t *autoStartList = (const auto_start_t *)(_Lauto_start);

void auto_start_init(void) {
    uint16_t length = &_Hauto_start - &_Lauto_start;
    uint8_t count = length / sizeof(auto_start_t);
    for (uint8_t i = 0; i < count; i++) {
        autoStartList[i].function();
    }
}
```

Usage in any module:

```c
// my_module.c
void my_module_init(void) { /* ... */ }
AUTO_START(my_module_init);
```

`auto_start_init()` called once in startup fires everything. No manual wiring.

## Why It Works

The linker collects all `AUTO_START` entries into a contiguous section. At runtime,
section bounds are known via linker symbols (`_Lauto_start`, `_Hauto_start`).
Iteration is trivial — just walk the array of function pointers.

This is the same architecture the Linux kernel uses for `__initcall`.

## Design Origin

Designed from first principles — no references consulted. Independently arrived
at the same solution as Linux.

## Current Status

**Not adopted.** Works on XC8 C90 (tested). Not adopted because:

1. **Opacity** — linker-section traversal is "eldritch dark magic" to anyone who
   doesn't know XC8 psect naming. Violates readability goal. (Though the "next
   maintainer" concern is relaxed — see below.)
2. **C99 unknown** — `__section()` attribute syntax is XC8 C90 specific. The
   C99 front-end (clang-based xc8-cc) likely uses different syntax. Untested.
3. **Solves a non-painful problem** — current approach of `#ifdef DEVELOPMENT`
   blocks in init functions is visible, obvious, and works fine.

## Reconsideration Trigger

The onboarding cost axis weakens if there's genuinely no next maintainer. The
remaining friction is: every new module requires manually wiring its init into
`system.c` across multiple projects. Autostart eliminates that entirely.

If C99 compatibility is confirmed (or C99 is definitively abandoned), and the
manual wiring friction becomes annoying enough across N projects, reconsider.

## Source

Original implementation: https://gist.github.com/DaelonSuzuka/593997566b4be21cf376c646c03a609a
