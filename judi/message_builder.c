#include "message_builder.h"
#include "json_node.h"
#include "json_print.h"
#include <stdint.h>
#include <string.h>

/* ************************************************************************** */
// special nodes, for use with the builder

const json_node_t endNode = {nControl, "\e"};
const json_node_t openBraceNode = {nControl, "{"};
const json_node_t closeBraceNode = {nControl, "}"};

/* ************************************************************************** */

typedef struct {
    json_node_t nodes[MESSAGE_LENGTH];
    uint8_t length;
} message_t;

static message_t message;

// clear the temporary message
void reset_message(void) {
    //
    memset(&message, 0, sizeof(message_t));
}

// add a single node to the temporary message
void add_node(const json_node_t node) {
    if (message.length < MESSAGE_LENGTH) {
        message.nodes[message.length++] = node;
    }
}

// add a list of nodes to the temporary message
void add_nodes(const json_node_t *nodes) {
    uint8_t i = 0;

    while (message.length < MESSAGE_LENGTH) {
        message.nodes[message.length++] = nodes[i++];

        if (nodes[i].type == nControl) {
            char c = ((const char *)nodes[i].contents)[0];
            if (c == '\e') {
                break;
            }
        }
    }
}

// terminate the message and send it to the specified print destination
void print_message(printer_t destination) {
    // make sure the node list is terminated
    add_node(endNode);

    // send the node list to the json printer
    json_print(destination, &message.nodes[0]);

    // clear the node list for next time
    reset_message();
}