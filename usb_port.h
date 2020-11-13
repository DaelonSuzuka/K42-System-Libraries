#ifndef _USB_PORT_H_
#define _USB_PORT_H_

#include "peripherals/uart.h"

/* ************************************************************************** */

extern void usb_port_init(uart_interface_t interface);

/* -------------------------------------------------------------------------- */

// Print a single character to the USB port
extern void usb_putch(char data);

// Read a single character from the USB port
extern char usb_getch(void);

// Print a string
extern void usb_print(const char *string);

// Print a string and append a newline
extern void usb_println(const char *string);

#endif // _USB_PORT_H_