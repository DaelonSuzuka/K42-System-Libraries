#include "timestamp.h"

/* ************************************************************************** */

// temp storage for the timestamp
static system_time_t timeCache;

// timestamp node list
const json_node_t timestamp[] = {
    {nKey, "time"},             //
    {nU32, (void *)&timeCache}, //
    {nControl, "\e"},           //
};

// make sure the temp timestamp is updated
const json_node_t *_get_timestamp(void) {
    timeCache = get_current_time();
    return &timestamp;
}

// json function wrapper node
const node_function_t get_timestamp = {_get_timestamp};

// prebuilt node for use with the message builder
const json_node_t timestampNode = {nFunction, (void *)&get_timestamp};