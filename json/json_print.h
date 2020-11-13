#ifndef _JSON_PRINT_H_
#define _JSON_PRINT_H_

/* ************************************************************************** */

/*  json_node.h contains the definition of the json_node_t data structure and
    its elements, as well as instructions and examples on how to construct your
    own C representations for JSON objects.
*/
#include "json_node.h"

/* ************************************************************************** */

/*  A pointer to a function that can print null-terminated strings. This allows
    the json_print() to not know about system features like serial port
    availability.

    Any function that can handle pointers to null-terminated strings is valid,
    whether it sends the string out a serial port, logs it to file, or even
    throws it away entirely.
*/
typedef void (*printer_t)(const char *);

/*  json_print() creates a JSON object using 'nodeList' and prints it using the
    provided 'destination' function pointer. This allows json_print() to target 
    different serial ports if the system has them.

    If always providing a second argument proves cumbersome, it's always 
    possible to write a wrapper macro that fills in the destination you want.

    Example:
    #define my_json_print(nodeList) json_print(fixedDestination, nodeList)

    json_print() provides several safety features to 
*/
extern void json_print(printer_t destination, const json_node_t *nodeList);

#endif // _JSON_PRINT_H_