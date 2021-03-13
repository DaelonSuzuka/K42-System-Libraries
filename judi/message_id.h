#ifndef _MESSAGE_ID_H_
#define _MESSAGE_ID_H_

#include "json_node.h"
#include "judi.h"
#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

// a shortcut to properly include the above node in your node list
#define MESSAGE_ID_NODE nFunction, (void *)&get_message_id

// prebuilt node for use with the message builder
extern const json_node_t messageIdNode;

// message id function node
// this will conditionally include the most recent message id
extern const node_function_t get_message_id;

/* -------------------------------------------------------------------------- */

// json node list to add ["message_id":<id>] to a judi message
// this will unconditionally include the most recent message id
// you probably don't want to use this directly
extern const json_node_t messageID[];

/* ************************************************************************** */

// scans the json buffer for a message id field and grabs the id if present
// call this at the end of judi message preprocessing
extern void grab_message_id(json_buffer_t *buf);

/* -------------------------------------------------------------------------- */

//* these manual control functions usually don't need to be called
//* message ids are typically handled automatically

// manually control whether the message id is sent with the next message
extern bool get_need_to_send(void);
extern void set_need_to_send(bool state);

// manually set the message id
extern void set_message_id(uint16_t id);

#endif // _MESSAGE_ID_H_