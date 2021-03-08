#ifndef _USB_H_
#define _USB_H_

#include "os/system_time.h"
#include "peripherals/uart.h"
#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

typedef enum {} jsmntype_t;

typedef struct {
    jsmntype_t type;
    int start;
    int end;
    int size;
    int hash;
#ifdef JSMN_PARENT_LINKS
    int parent;
#endif
} jsmntok_t;

// Incoming JSON objects MUST BE shorter than this length
#define JSON_BUFFER_SIZE 256

// Maximum number of tokens jsmn can parse
#define MAX_TOKENS 64

typedef struct {
    char data[JSON_BUFFER_SIZE]; // The incoming JSON text goes here
    uint8_t length;              // The number of recieved bytes
    uint8_t depth;               // The current {} nesting level
    jsmntok_t tokens[MAX_TOKENS];
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

/* ************************************************************************** */
// Message buffer traversal utilities

// helper macros to look up token fields
#define TOKEN(number) &buf->data[buf->tokens[number].start]
#define HASH(number) buf->tokens[number].hash
#define TYPE(number) buf->tokens[number].type
#define PARENT(number) buf->tokens[number].parent
#define SIZE(number) buf->tokens[number].size
#define START_PTR(number) &buf->data[buf->tokens[number].start]
#define END_PTR(number) &buf->data[buf->tokens[number].end]

// the index in json_buffer_t.tokens of the top level json object
#define ROOT_OBJECT 0

// return the index of the token matching the given hash
// only searches inside the given json object
extern uint8_t find_key(json_buffer_t *buf, int8_t obj, int8_t hash);

/* ************************************************************************** */

// function pointer definition
typedef void (*responder_t)(json_buffer_t *buf);

// initialize the USB port by creating and passing in a UART interface object
extern void usb_init(uart_config_t *config, responder_t responder);

// returns true if a message is currently being recieved
extern bool usb_is_recieving(void);

// call this often to service the USB port
extern void usb_update(void);

#endif // _USB_H_