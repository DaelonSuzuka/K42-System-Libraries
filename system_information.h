#ifndef _SYSTEM_INFORMATION_H_
#define _SYSTEM_INFORMATION_H_

#include <stdint.h>

/* ************************************************************************** */
/*  System information

    Various information about the system, made available at runtime.
*/

// product information
extern const char productName[];
extern const char productVersion[];

// product software version

// compilation information
extern const uint16_t xc8Version;
extern const char processorModel[];
extern const char compileDate[];
extern const char compileTime[];

#endif // _SYSTEM_INFORMATION_H_