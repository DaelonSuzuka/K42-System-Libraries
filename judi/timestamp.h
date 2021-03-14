#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "json_node.h"
#include "judi.h"
#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

// a shortcut to properly include the above node in your node list
#define TIMESTAMP_NODE nFunction, (void *)&get_timestamp

// prebuilt node for use with the message builder
extern const json_node_t timestampNode;

// timestamp function node
extern const node_function_t get_timestamp;

/* ************************************************************************** */

#endif // _TIMESTAMP_H_