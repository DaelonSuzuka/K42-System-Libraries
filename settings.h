#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>

/* ************************************************************************** */

extern void settings_init(void);

extern uint8_t settings_create_group(uint8_t size, uint8_t slots);

extern void settings_load(uint8_t group, uint8_t *data);

extern void settings_save(uint8_t group, uint8_t *data);

#endif