#include "message_id.h"
#include "os/judi/hash.h"
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

static bool needToSendID = true;

bool get_need_to_send(void) {
    return needToSendID; //
}

void set_need_to_send(bool state) {
    needToSendID = state; //
}

/* -------------------------------------------------------------------------- */

// internal storage of current message id
static uint16_t _messageID = 0;

void set_message_id(uint16_t id) {
    _messageID = id;
    needToSendID = true;
}

/* -------------------------------------------------------------------------- */

// json node list to add ["message_id":<id>] to a judi message
const json_node_t messageID[] = {
    {nKey, "message_id"},        //
    {nU16, (void *)&_messageID}, //
    {nControl, "\e"},            //
};

/* -------------------------------------------------------------------------- */

// 'private' function to optionally print the message id
const json_node_t *_get_message_id(void) {
    if (needToSendID) {
        needToSendID = false;
        return &messageID;
    } else {
        return NULL;
    }
}

// json function wrapper node
const node_function_t get_message_id = {_get_message_id};

// prebuilt node for use with the message builder
const json_node_t messageIdNode = {nFunction, (void *)&get_message_id};

/* ************************************************************************** */

// scans the json buffer for a message id field and grabs the id if present
void grab_message_id(json_buffer_t *buf) {
    needToSendID = false;

    // grab message id
    uint8_t msg_id = find_key(buf, ROOT_OBJECT, hash_message_id);
    if (msg_id) {
        set_message_id(atoi(TOKEN(msg_id + 1)));
    }
}