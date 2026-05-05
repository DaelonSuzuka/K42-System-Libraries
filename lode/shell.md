# Shell Subsystem — Chitin v0.6.0

Interactive command shell for development builds. Disabled in release builds via preprocessor.
Fully non-blocking except for one 5ms busy-wait in escape sequence interception.

## Architecture

```mermaid
flowchart LR
    UART[UART Input] --> Shell[shell.c]
    Shell --> History[shell_history.c]
    Shell --> Cursor[shell_cursor.c]
    Shell --> Keys[shell_keys.c]
    Shell --> Processor[shell_command_processor.c]
    Processor --> Builtins[shell_builtins.c]
    Processor --> Commands[Registered Commands]
```

## Three Operating Modes

1. **Line mode** (default) — characters accumulate in `shell_line_t`, cursor moves, backspace deletes, history navigates. Standard readline-like behavior. Every operation is O(1) per character.

2. **Callback mode** — `shell_register_callback(fn)` redirects all input to a custom function. Callback returns 0 to continue, -1 to exit. Ctrl-C (char 3) always terminates. Used for interactive programs like `reboot` confirmation.

3. **TUI callback mode** — same mechanism with `shellCallbackSettings.fullscreen = 1`. On exit, `terminate_current_program()` resets terminal state (colors, screen, cursor) because "there's no way to know what state a program was in when we kill it." Programs don't have to clean up after themselves.

The callback system inverts the flow — instead of the shell calling your program, your program takes over the input stream. Same pattern as `ui_idle_block()` in the superloop.

## Escape Sequence Decoder

`shell_keys.c` decodes VT102/xterm escape sequences using sequence length + character positions. `intercept_escape_sequence()` busy-waits up to 5ms after ESC, collecting characters via `getch()` and trying `decode_escape_sequence()` after each one. Handles variant encodings (HOME has two sequences, F5 has two, etc.). Modifier keys (shift/alt/ctrl) extracted from positions 3-4 in longer sequences.

**Diagnostics mode** (F6 toggle) prints every decoded sequence — essential for terminal compatibility debugging.
**History inspection** (F7 toggle) dumps the ring buffer state, pointer position, and temp line — watch the history ring operate in real time.

## Key Design Decisions

- **X-macro code generation** — `KEY_NAME_LIST` and `KEY_MODIFIER_LIST` produce both the enum and the diagnostic string tables. No synchronization risk.
- **PIC18 paged memory workaround** — history lines are separate globals (`history_0`, `history_1`, ...) tied together by a pointer array. Can't have a struct array spanning memory pages. The "this is disgusting" comment in `shell_history.c` is honest.
- **`version -j`** — `sh_version` with `-j` prints device info as JSON via the node builder. Shell and JUDI share the same outbound serializer.
- **`shell_command_processor.h` guard** — uses `#ifdef LOGGING_ENABLED` but should be `#ifdef SHELL_ENABLED`. Commands can't register when shell is disabled, but the guard checks the wrong flag.

## Known Issues

See `bug-register.md` for: `insert_char_at_cursor` uint8_t wrap (critical), history modulo (medium), `isprint`/`iscntrl` signed char UB (medium), `shell_utils.h` printf vs sh_print (low), command registration no bounds check (high).

**5ms blocking loop** in `intercept_escape_sequence()` — the only place the shell blocks. Can cause missed deadlines (button debounce, display, RF). Fix requires a persistent state machine across `shell_update()` calls. The blocking also prevents host-machine compilation/testing — `getch()` is platform-specific. A stateful fix would naturally enable host testing. See `lode/roadmap.md`.

## Enabling the Shell

Add to project's build defines:

```c
// In project's shell_config.h
#define SHELL_ENABLED
#define SHELL_HISTORY_ENABLED
#define SHELL_MAX_LENGTH 64
```

## Command Registration

Register commands in your module's `_init()` function:

```c
void my_module_init(void) {
    // ... module setup ...
    
#ifdef SHELL_ENABLED
    shell_register_command(sh_mycommand, "mycommand");
#endif
}
```

## Command Signature

All shell commands use this signature:

```c
void command_name(int argc, char **argv);
```

## Pattern 1: One-Shot Commands

Execute immediately and return. Parse arguments with `argc`/`argv`:

```c
void sh_mycommand(int argc, char **argv) {
    switch (argc) {
    case 1:
        mycommand_print_usage();
        return;
    case 2:
        if (!strcmp(argv[1], "list")) {
            mycommand_list_items();
            return;
        }
        break;
    case 3:
        if (!strcmp(argv[1], "set")) {
            uint16_t value = atoi(argv[2]);
            mycommand_set(value);
            return;
        }
        break;
    }
    println("invalid arguments");
}
```

## Pattern 2: Interactive Callbacks

Enter a monitoring/control mode until user exits:

```c
int8_t my_callback(char currentChar) {
    if (currentChar == 3) {  // Ctrl-C exits
        return -1;
    }
    
    // Handle key input
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        switch (key.key) {
        case UP:
            // increment something
            display_value();
            return 0;
        case DOWN:
            // decrement something
            display_value();
            return 0;
        }
    }
    
    return 0;  // continue running callback
}

void sh_mycommand(int argc, char **argv) {
    println("entering mycommand mode (Ctrl-C to exit)");
    shell_register_callback(my_callback);
}
```

**Callback return values:**
- `0` = continue running callback
- `-1` = exit callback mode, return to shell

## Pattern 3: Full TUI Interface

For text-based user interfaces (menus, dashboards, editors), use callback mode with terminal control sequences.

### Terminal Control Functions

From `shell_utils.h`:

```c
// Screen control
term_reset_screen()       // Clear entire screen, cursor to 0,0
term_clear_to_right()     // Clear from cursor to end of line
term_hide_cursor()        // Hide cursor during TUI
term_show_cursor()        // Show cursor again
term_cursor_set(x, y)     // Move cursor to column x, row y
term_cursor_home()        // Move cursor to column 0
term_cursor_up(n)         // Move cursor up n rows
term_cursor_down(n)       // Move cursor down n rows
term_cursor_left(n)       // Move cursor left n columns
term_cursor_right(n)      // Move cursor right n columns
```

### Text Colors and Attributes

From `shell_colors.h`:

```c
// Reset to default
TXT_RESET                 // White on black

// Text colors
TXT_RED, TXT_GREEN, TXT_BLUE, TXT_YELLOW, TXT_CYAN, TXT_MAGENTA, TXT_WHITE

// Bright variants
TXT_BR_RED, TXT_BR_GREEN, TXT_BR_BLUE, ...

// Bold variants
TXT_BOLD_RED, TXT_BOLD_GREEN, ...

// Background colors
TXT_BG_RED, TXT_BG_GREEN, TXT_BG_BLUE, ...

// Effects
TXT_BOLD, TXT_UNDERLINE

// Invert (swap foreground/background)
invert_text_attribute()   // From shell_utils.h
```

### Key Detection

Keys are identified using `identify_key()`:

```c
key_t key = identify_key(currentChar);

// Available keys (from shell_keys.h)
// UP, DOWN, LEFT, RIGHT
// HOME, END, INSERT, DELETE
// PAGEUP, PAGEDOWN
// F1 through F12
// ENTER, ESCAPE, TAB, BACKSPACE
// UNKNOWN

// Check for modifier keys
if (key.mod == CTRL) { ... }
if (key.mod == SHIFT) { ... }
```

### TUI Structure

A full TUI has three parts: setup, draw, and callback.

```c
// State for TUI
static uint8_t selected_line = 0;
static uint8_t selected_value = 0;

// Initialize state, then enter callback mode
void sh_mytui(int argc, char **argv) {
    selected_line = 0;
    selected_value = DEFAULT_VALUE;
    
    term_hide_cursor();
    draw_mytui();
    
    shell_register_callback(mytui_callback);
}

// Refresh the entire screen
void draw_mytui(void) {
    term_reset_screen();
    term_cursor_set(0, 0);
    
    print_mytui_header();
    print_mytui_list();
    print_mytui_footer();
    
    // Position cursor on selected line
    term_cursor_set(0, selected_line + HEADER_ROWS);
    invert_text_attribute();
    print_current_line();
}
```

### TUI Callback Pattern

```c
int8_t mytui_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        
        switch (key.key) {
        case UP:
            if (selected_line > 0) {
                reprint_line();          // Unhighlight current
                selected_line--;
                term_cursor_up(1);
                invert_text_attribute();
                reprint_line();          // Highlight new
            }
            return 0;
            
        case DOWN:
            if (selected_line < MAX_LINES - 1) {
                reprint_line();
                selected_line++;
                term_cursor_down(1);
                invert_text_attribute();
                reprint_line();
            }
            return 0;
            
        case ENTER:
            // Save and exit
            save_current_value();
            return -1;  // Exit callback mode
            
        case ESCAPE:
            // Exit without saving
            return -1;
            
        case F5:
            // Refresh screen
            draw_mytui();
            return 0;
        }
    }
    
    return 0;  // Continue running
}
```

### Complete TUI Example: Menu Editor

```c
// State
static uint8_t cursor_row = 0;
static uint8_t values[10];
static const uint8_t num_items = 10;

void draw_menu(void) {
    term_reset_screen();
    term_cursor_set(0, 0);
    
    println("=== Configuration Menu ===");
    println("");
    
    for (uint8_t i = 0; i < num_items; i++) {
        if (i == cursor_row) {
            invert_text_attribute();
        }
        printf("Item %d: [%3d]  %s\r\n", i, values[i], 
               i == cursor_row ? "<--" : "   ");
        if (i == cursor_row) {
            reset_text_attributes();
        }
    }
    
    println("");
    println("UP/DOWN: navigate  LEFT/RIGHT: change value");
    println("ENTER: save  ESC: cancel");
    
    // Position cursor
    term_cursor_set(0, cursor_row + 3);
}

int8_t menu_callback(char currentChar) {
    if (!iscntrl(currentChar)) return 0;
    
    key_t key = identify_key(currentChar);
    
    switch (key.key) {
    case UP:
        if (cursor_row > 0) cursor_row--;
        draw_menu();
        return 0;
    case DOWN:
        if (cursor_row < num_items - 1) cursor_row++;
        draw_menu();
        return 0;
    case LEFT:
        if (values[cursor_row] > 0) values[cursor_row]--;
        draw_menu();
        return 0;
    case RIGHT:
        if (values[cursor_row] < 255) values[cursor_row]++;
        draw_menu();
        return 0;
    case ENTER:
        term_reset_screen();
        term_show_cursor();
        println("Values saved!");
        return -1;
    case ESCAPE:
        term_reset_screen();
        term_show_cursor();
        println("Cancelled.");
        return -1;
    }
    return 0;
}

void sh_menuedit(int argc, char **argv) {
    term_hide_cursor();
    draw_menu();
    shell_register_callback(menu_callback);
}
```

## Standard Includes for Commands

```c
#include "os/serial_port.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "os/shell/shell_command_utils.h"
#include "os/shell/shell_keys.h"
#include "os/shell/shell_utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
```

## Key Files

| File | Purpose |
|------|---------|
| `shell.c` | Main shell input loop |
| `shell_command_processor.c` | Parses and dispatches commands |
| `shell_builtins.c` | Built-in commands (help, clear, etc.) |
| `shell_history.c` | Command history navigation |
| `shell_keys.c` | Key code definitions and parsing |
| `shell_cursor.c` | Cursor movement within line editor |
| `shell_utils.h` | Terminal control macros (cursor, clear) |
| `shell_colors.h` | ANSI color and attribute codes |
| `shell_config.h` | Project-specific configuration |