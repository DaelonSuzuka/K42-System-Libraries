# JUDI Protocol

JSON UART Device Interface - A JSON-based protocol for structured USB/serial communication.

## Overview

JUDI provides structured messaging over UART using JSON. It's typically used in development builds for external configuration and monitoring, but can be included in release builds for USB-enabled devices.

## Message Buffer

```c
typedef struct {
    char data[JSON_BUFFER_SIZE];   // Configurable, typically 256 bytes
    uint8_t length;
    uint8_t depth;                 // JSON nesting level
    jsmntok_t tokens[MAX_TOKENS];  // Parsed tokens (jsmn parser)
    int tokensParsed;
    system_time_t messageStartTime;
    system_time_t lastCharacterTime;
    union {
        struct {
            unsigned timedout : 1;
            unsigned stalled : 1;
        };
        uint8_t errors;
    };
} json_buffer_t;
```

## Token Access Macros

```c
TOKEN(n)       // Pointer to token data
HASH(n)        // Token hash value (for fast key matching)
TYPE(n)        // Token type (object, array, string, etc.)
PARENT(n)      // Parent token index
SIZE(n)        // Token size
```

## Initialization and Update

```c
// Initialization with response handler
void judi_init(void (*responder)(json_buffer_t *buf));

// Check if message is being received
bool judi_is_receiving(void);

// Poll in main loop - returns true when complete message received
bool judi_update(char currentChar);
```

## Key Lookup Pattern

```c
// Parse incoming JSON
int8_t key_idx = find_key(buf, ROOT_OBJECT, hash_value);
if (key_idx != -1) {
    // Found the key - extract value
    char *value = TOKEN(key_idx + 1);
    // process value...
}
```

The `hash_value` is pre-computed using the hash function for fast string matching.

## Message Building

Use `message_builder.c` to construct outgoing messages:

```c
reset_message();                        // Clear temp buffer
add_node(openBraceNode);                // Start JSON object
add_node(my_node);                      // Add pre-defined node
add_node(closeBraceNode);                // End JSON object
print_message(usb_printer);             // Send via destination printer
```

Nodes are constructed using `json_node_t` from `json_node.h`.

## Key Files

| File | Purpose |
|------|---------|
| `judi.c` | Main JUDI implementation |
| `judi_messages.c` | Message definitions and handlers |
| `message_builder.c` | Construct outgoing messages |
| `hash_function.c` | Fast string hashing for key lookup |
| `timestamp.c` | Message timestamping |

## Dependencies

- `jsmn` - JSON parser (included in `os/json/`)
- `system_time` - Millisecond timestamps
- UART interface (passed from project)