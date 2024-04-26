#include "system_information.h"

/* ************************************************************************** */
/*  System information

    Various information about the system, made available at runtime.
*/

#define xstr(s) str(s)
#define str(s) #s

// product name
const char productName[] = xstr(__PRODUCT_NAME__);

// product software version
const char productVersion[] = xstr(__PRODUCT_VERSION__);

// compilation information
const uint16_t xc8Version = __XC8_VERSION;
const char processorModel[] = xstr(__PROCESSOR__);
const char compileDate[] = __DATE__;
const char compileTime[] = __TIME__;