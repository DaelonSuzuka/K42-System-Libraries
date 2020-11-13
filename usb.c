#ifdef USB_ENABLED

#include "usb.h"
#include "libraries/str_len.h"
#include "os/json/jsmn.h"
#include "os/json/json_messages.h"
#include "os/json/json_print.h"
#include "os/log_macros.h"
#include "os/serial_port.h"
#include "os/usb_port.h"
#include "peripherals/uart.h"
#include "usb/hash.h"
#include "usb/messages.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

static json_buffer_t buffer[2];
static uint8_t active = 0;

void reset_json_buffer(json_buffer_t *buffer) {
    memset(buffer, 0, sizeof(json_buffer_t));
}

void swap_active_buffer(void) {
    // swap which buffer is active
    if (active == 0) {
        active = 1;
    } else {
        active = 0;
    }

    // wipe the new buffer before using it
    reset_json_buffer(&buffer[active]);
}

/* ************************************************************************** */

static responder_t response_function;

void usb_init(uart_interface_t interface, responder_t responder) {
    // pass the uart down to the usb port driver
    usb_port_init(interface);

    response_function = responder;

    reset_json_buffer(&buffer[0]);
    reset_json_buffer(&buffer[1]);

    reset_message();

    log_register();
}

bool usb_is_recieving(void) {
    if (buffer[active].depth > 0) {
        return true;
    }
    return false;
}

/* ************************************************************************** */

void message_timeout_error(void) {
    LOG_INFO({ println("message_timeout_error"); });
    // It's been too long since the current message started.
    // We should scan what we've received for a message ID, then send back
    // an error message, then reset our buffer and try again
}

void message_stall_error(void) {
    LOG_INFO({ println("message_stall_error"); });
    // It's been too long since the last character was rx'd
    // We should scan what we've received for a message ID, then send back
    // an error message, then reset our buffer and try again
}

/* -------------------------------------------------------------------------- */

#define MESSAGE_MAXIMUM_TIME 100
#define MESSAGE_TIMEOUT_WINDOW 100

void insert_character(json_buffer_t *buf, char currentChar) {
    if ((currentChar == '{') && (buf->depth == 0)) {
        LOG_INFO({ println("Message start"); });
        buf->messageStartTime = get_current_time();
        buf->lastCharacterTime = get_current_time();
    }

    if (time_since(buf->messageStartTime) > MESSAGE_MAXIMUM_TIME) {
        buf->timedout = true;
    }
    if (time_since(buf->lastCharacterTime) > MESSAGE_TIMEOUT_WINDOW) {
        buf->stalled = true;
    }

    if (currentChar == '{') {
        buf->depth++;
    }

    if (buf->depth > 0) {
        buf->data[buf->length++] = currentChar;
        buf->lastCharacterTime = get_current_time();
    }

    if (currentChar == '}' && buf->depth > 0) {
        buf->depth--;
    }
}

/* -------------------------------------------------------------------------- */

void preprocess(json_buffer_t *buf) {
    jsmn_parser parser;
    jsmn_init(&parser);

    // tokenize the buffer
    buf->tokensParsed = jsmn_parse(&parser, buf->data, str_len(buf->data),
                                   buf->tokens, MAX_TOKENS);

    // terminate each token in the original string
    for (uint8_t i = 0; i < buf->tokensParsed; i++) {
        buf->data[buf->tokens[i].end] = 0;
    }

    // hash the tokens
    for (uint8_t i = 0; i < buf->tokensParsed; i++) {
        buf->tokens[i].hash = buf->tokens[i].type;

        if (buf->tokens[i].type == JSMN_STRING) {
            int hash = hash_string(TOKEN(i));

            if (hash != -1) {
                buf->tokens[i].hash = hash;
            }
        }
    }
}

/* ************************************************************************** */

void usb_update(void) {
    char currentChar = usb_getch();

    // return early if we don't have a valid character
    if (!isprint(currentChar)) {
        return;
    }

    insert_character(&buffer[active], currentChar);

    // These conditions mean we've reached the end of a JSON object
    if ((buffer[active].length > 0) && (buffer[active].depth == 0)) {
        // add terminating null, after closing brace
        buffer[active].data[buffer[active].length] = 0;

        system_time_t time;

        LOG_INFO({
            print("Message complete in ");
            time = time_since(buffer[active].messageStartTime);
            printf("%lu mS\r\n", time);
        });
        LOG_DEBUG({
            print("Message: [");
            print(&buffer[active].data);
            println("]");
        });

        preprocess(&buffer[active]);
        LOG_INFO({
            print("Preprocessing completed in: ");
            time = time_since(time);
            printf("%lu mS\r\n", time);
        });

        response_function(&buffer[active]);
        LOG_INFO({
            print("Response completed in: ");
            time = time_since(time);
            printf("%lu mS\r\n", time);
        });

        swap_active_buffer();
    }
}

#endif