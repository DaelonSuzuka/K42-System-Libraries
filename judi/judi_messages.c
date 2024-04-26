#include "judi_messages.h"
#include "message_id.h"
#include "os/system_information.h"
#include "peripherals/device_information.h"
#include "system.h"
#include <string.h>

/* ************************************************************************** */
// message preambles

const json_node_t requestPreamble[] = {
    {nControl, "{"},   //
    {MESSAGE_ID_NODE}, //
    {nKey, "request"}, //
    {nControl, "{"},   //
    {nControl, "\e"},  //
};

const json_node_t updatePreamble[] = {
    {nControl, "{"},   //
    {MESSAGE_ID_NODE}, //
    {nKey, "update"},  //
    {nControl, "{"},   //
    {nControl, "\e"},  //
};

const json_node_t responsePreamble[] = {
    {nControl, "{"},    //
    {MESSAGE_ID_NODE},  //
    {nKey, "response"}, //
    {nControl, "{"},    //
    {nControl, "\e"},   //
};

/* ************************************************************************** */
// standard responses

const json_node_t responseOk[] = {
    {nControl, "{"},    //
    {MESSAGE_ID_NODE},  //
    {nKey, "response"}, //
    {nString, "ok"},    //
    {nControl, "\e"},   //
};

const json_node_t responseError[] = {
    {nControl, "{"},    //
    {MESSAGE_ID_NODE},  //
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