#ifndef _JSON_MESSAGES_H_
#define _JSON_MESSAGES_H_

#include "json_node.h"

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