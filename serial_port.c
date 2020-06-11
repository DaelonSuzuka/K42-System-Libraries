#include "serial_port.h"
#include "peripherals/uart.h"

/* ************************************************************************** */

EMPTY_UART_INTERFACE(uart);

void serial_port_init(uart_interface_t interface) {
    //
    uart = interface;
}

/* -------------------------------------------------------------------------- */

// Print a single character to the console
// Also needed for compiler provided printf
void putch(char data) {
    //
    uart.tx_char(data);
}

// Read a single character from the console
char getch(void) {
    //
    return uart.rx_char();
}

// Print a string
void print(const char *string) {
    /*  Wrap UART_TX_STRING() to provide both a portability layer, and to keep
        the user from having to add the termination character every time they
        print
    */
    uart.tx_string(string, '\0');
}

// Print a string and append a newline
void println(const char *string) {
    uart.tx_string(string, '\0');
    print("\r\n");
}