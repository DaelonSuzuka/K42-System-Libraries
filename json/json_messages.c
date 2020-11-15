#include "json_messages.h"
#include "peripherals/device_information.h"
#include "system.h"
#include <string.h>

/* ************************************************************************** */
typedef struct {
    json_node_t nodes[MESSAGE_LENGTH];
    uint8_t length;
} message_t;

message_t message;

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

/* ************************************************************************** */
// Special node definitions

const json_node_t endNode = {nControl, "\e"};
const json_node_t openBraceNode = {nControl, "{"};
const json_node_t closeBraceNode = {nControl, "}"};

/* ************************************************************************** */
// message preambles

const json_node_t requestPreamble[] = {
    {nControl, "{"},   //
    {nKey, "request"}, //
    {nControl, "{"},   //
    {nControl, "\e"},  //
};

const json_node_t updatePreamble[] = {
    {nControl, "{"},  //
    {nKey, "update"}, //
    {nControl, "{"},  //
    {nControl, "\e"}, //
};

const json_node_t responsePreamble[] = {
    {nControl, "{"},    //
    {nKey, "response"}, //
    {nControl, "{"},    //
    {nControl, "\e"},   //
};

/* ************************************************************************** */
// standard responses

const json_node_t responseOk[] = {
    {nControl, "{"},    //
    {nKey, "response"}, //
    {nString, "ok"},    //
    {nControl, "\e"},   //
};

const json_node_t responseError[] = {
    {nControl, "{"},    //
    {nKey, "response"}, //
    {nString, "error"}, //
    {nControl, "\e"},   //
};

/* ************************************************************************** */
// message components

/*  This unit's unique serial number, programmed at the factory by Microchip
    "serial" : <hexMUI>
*/
const json_node_t serial_number[] = {
    {nKey, "serial_number"},    //
    {nString, (void *)&hexMUI}, //
    {nControl, "\e"},           //
};

/*  The version of the shell code

    "compiler_version" : <__XC8_VERSION>,
    "compile_date" : <__DATE__>,
    "compile_time" : <__TIME__>
*/
const json_node_t compilation_info[] = {
    {nKey, "compiler_version"},      //
    {nU16, (void *)&xc8Version},     //
    {nKey, "compile_date"},          //
    {nString, (void *)&compileDate}, //
    {nKey, "compile_time"},          //
    {nString, (void *)&compileTime}, //
    {nControl, "\e"},                //
};

/* ************************************************************************** */
// device info components

const json_node_t deviceInfo[] = {
    {nKey, "device_info"},               //
    {nControl, "{"},                     //
    {nKey, "product_name"},              //
    {nString, (void *)&productName},     //
    {nNodeList, (void *)&serial_number}, //
    {nKey, "firmware_version"},          //
    {nString, (void *)&productVersion},  //
    {nKey, "protocol_version"},          //
    {nString, "1.0.0"},                  //
    {nControl, "\e"},                    //
};