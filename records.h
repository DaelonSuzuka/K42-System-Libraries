#ifndef _RECORDS_H_
#define _RECORDS_H_

#include <stdint.h>

/* ************************************************************************** */

// initialize the record storage module
// minAddress and maxAddress allow the user to define a keep-out zone
// default minAddress should be 0
// setting maxAddress of 0 means 'no limit'
extern void records_init(uint16_t minAddress, uint16_t maxAddress);

//
enum recordLocation { record_eeprom, record_flash };

// allocate a new chunk
// returns the chunkID of the created chunk
// recordSize is the length of the bytearray you want to be stored
// overprovisionFactor is 
// overprovisionFactor 
// overprovisionFactor of 0 means no wear leveling
// location is what type of storage to put the record in
extern uint8_t create_new_record(uint16_t recordSize,
                                 uint16_t overprovisionFactor,
                                 enum recordLocation location);

// read the specified chunk out to the destination
extern void load_record(uint8_t recordID, uint8_t *destination);

// writes the provided data into the specified chunk
extern void store_record(uint8_t recordID, uint8_t *source);

#endif