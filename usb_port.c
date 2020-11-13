#include "usb_port.h"
#include "peripherals/uart.h"

/* ************************************************************************** */

EMPTY_UART_INTERFACE(uart);

void usb_port_init(uart_interface_t interface) {
    //
    uart = interface;
}

/* -------------------------------------------------------------------------- */

// Print a single character to the USB port
void usb_putch(char data) { uart.tx_char(data); }

// Read a single character from the USB port
char usb_getch(void) { return uart.rx_char(); }

// Print a string to the USB port
void usb_print(const char *string) { uart.tx_string(string, '\0'); }

// Print a string and append a newline
void usb_println(const char *string) {
    uart.tx_string(string, '\0');
    usb_print("\r\n");
}