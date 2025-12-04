#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

/* ************************************************************************** */

#include "peripherals/uart.h"
#include <stdio.h>

/* ************************************************************************** */

// Setup
extern void serial_port_init(uart_config_t *config);

/* -------------------------------------------------------------------------- */

// Print a single character to the serial port
// Also needed for compiler provided printf
extern void putch(char data);

// annoyingly, getch return type changes between c89 and c99
#if defined __XC8_CC_C99__ || defined __XC8_C99__
#define GETCH_RETURN_TYPE int
#else
#define GETCH_RETURN_TYPE char
#endif

// Read a single character from the console
extern GETCH_RETURN_TYPE getch(void);

// Print a string
extern void print(const char *string);

// Print a string and append a newline
extern void println(const char *string);

/* ************************************************************************** */

#endif // _SERIAL_PORT_H_