#ifndef _JSON_MESSAGES_H_
#define _JSON_MESSAGES_H_

#include "json_node.h"
#include "json_print.h"
#include <stdint.h>

/* ************************************************************************** */
// configuration

// max number of nodes
#define MESSAGE_LENGTH 32

/* ************************************************************************** */

// clear the temporary message
extern void reset_message(void);

// add a single node to the temporary message
extern void add_node(const json_node_t node);

// add a list of nodes to the temporary message
extern void add_nodes(const json_node_t *nodes);

// terminate the message and send it to the specified print destination
extern void print_message(printer_t destination);

/* ************************************************************************** */
// special node definitions

extern const json_node_t endNode;
extern const json_node_t openBraceNode;
extern const json_node_t closeBraceNode;

/* ************************************************************************** */
// message id stuff

extern uint16_t _messageID;
extern const json_node_t messageID[];

/* ************************************************************************** */
// message preambles

extern const json_node_t requestPreamble[];
extern const json_node_t updatePreamble[];
extern const json_node_t responsePreamble[];

/* ************************************************************************** */
// standard responses

extern const json_node_t responseOk[];
extern const json_node_t responseError[];

/* ************************************************************************** */
// message components

extern const json_node_t serial_number[];
extern const json_node_t software_version[];
extern const json_node_t compilation_info[];


extern const json_node_t deviceInfo[];

#endif // _JSON_MESSAGES_H_