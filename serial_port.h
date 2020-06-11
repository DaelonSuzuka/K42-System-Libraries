#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

/* ************************************************************************** */

#include "peripherals/uart.h"
#include <stdio.h>

/* ************************************************************************** */

// Setup
extern void serial_port_init(uart_interface_t interface);

/* -------------------------------------------------------------------------- */

// Print a single character to the serial port
// Also needed for compiler provided printf
extern void putch(char data);

// Read a single character from the console
extern char getch(void);

// Print a string
extern void print(const char *string);

// Print a string and append a newline
extern void println(const char *string);

/* ************************************************************************** */

#endif // _SERIAL_PORT_H_