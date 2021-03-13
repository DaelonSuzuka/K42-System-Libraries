#ifndef _MESSAGE_BUILDER_H_
#define _MESSAGE_BUILDER_H_

#include "json_node.h"
#include "json_print.h"

/* ************************************************************************** */
// configuration

// max number of nodes
#define MESSAGE_LENGTH 32

/* ************************************************************************** */
// special nodes, for use with the builder

extern const json_node_t endNode;
extern const json_node_t openBraceNode;
extern const json_node_t closeBraceNode;

/* ************************************************************************** */

// clear the temporary message
extern void reset_message(void);

// add a single node to the temporary message
extern void add_node(const json_node_t node);

// add a list of nodes to the temporary message
extern void add_nodes(const json_node_t *nodes);

// terminate the message and send it to the specified print destination
extern void print_message(printer_t destination);

#endif // _MESSAGE_BUILDER_H_