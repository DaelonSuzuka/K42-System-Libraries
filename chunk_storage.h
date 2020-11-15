#ifndef _CHUNK_STORAGE_H_
#define _CHUNK_STORAGE_H_

#include <stdint.h>

/* ************************************************************************** */

// initialize the chunk storage module
// minAddress and maxAddress allow the user to define a keep-out zone
// default minAddress should be 0
// setting maxAddress of 0 means 'no limit'
extern void chunk_storage_init(uint16_t minAddress, uint16_t maxAddress);

// allocate a new chunk
// returns the chunkID of the created chunk
// chunkSize is the length of the bytearray you want to be stored
// overprovisionSize is the number of bytes you want allocated to wear-leveling
// overprovisionSize should be a multiple of chunkSize
// overprovisionSize of 0 means no wear leveling
extern uint8_t create_new_chunk(uint16_t chunkSize, uint16_t overprovisionSize);

// read the specified chunk out to the destination
extern void read_chunk(uint8_t chunkID, uint8_t *destination);

// writes the provided data into the specified chunk
extern void write_chunk(uint8_t chunkID, uint8_t *source);

#endif