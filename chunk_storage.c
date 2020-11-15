#include "chunk_storage.h"
#include "os/logging.h"
#include "peripherals/nonvolatile_memory.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

#define MAX_CHUNK_SIZE 20
#define MAXIMUM_NUMBER_OF_CHUNKS 10

typedef struct {
    uint16_t start;
    uint16_t size;
    uint16_t overprovision;
} chunk_metadata_t;

chunk_metadata_t chunks[MAXIMUM_NUMBER_OF_CHUNKS];
uint8_t numOfChunks = 0;

void print_chunk(uint8_t chunkID) {
    printf("ID: %d start: %3d, size: %3d, length: %3d op: %3d \n", chunkID,
           chunks[chunkID].start, chunks[chunkID].size,
           chunks[chunkID].size + chunks[chunkID].overprovision,
           chunks[chunkID].overprovision);
}

void print_chunks(void) {
    for (uint8_t i = 0; i < MAXIMUM_NUMBER_OF_CHUNKS; i++) {
        print_chunk(i);
    }
    printf("\n");
}

/* ************************************************************************** */

void chunk_storage_init(uint16_t minAddress, uint16_t maxAddress) {
    // clear all the chunk data
    for (uint8_t i = 0; i < MAXIMUM_NUMBER_OF_CHUNKS; i++) {
        chunks[i].size = 0;
        chunks[i].overprovision = 0;
        chunks[i].start = 0;
    }

    // make the first chunk start at the minimum allowed address
    chunks[0].start = minAddress;

    // TODO: implement maxAddress

    log_register();
}

uint8_t create_new_chunk(uint16_t chunkSize, uint16_t overprovisionSize) {
    chunks[numOfChunks].size = chunkSize;
    chunks[numOfChunks].overprovision = overprovisionSize;

    uint16_t nextStart =
        chunks[numOfChunks].start + chunkSize + overprovisionSize;

    numOfChunks++;
    chunks[numOfChunks].start = nextStart;

    return numOfChunks;
}

/* ************************************************************************** */

#define NO_VALID_RECORD -1

/*  find_current_record() steps through EEPROM, reading the first byte of
    every stored record, looking for one where the first bit is 0.
*/
int16_t find_current_record(uint8_t chunkID) {
    uint8_t address = chunks[chunkID].start;

    while (address < (chunks[chunkID].size * chunks[chunkID].overprovision)) {
        if ((internal_eeprom_read(address) & 0x80) == 0) {
            return address;
        }
        address += chunks[chunkID].size;
    }

    // didn't find record
    return NO_VALID_RECORD;
}

/* ************************************************************************** */

void read_record(uint16_t address, uint8_t chunkID, uint8_t *destination) {
    destination[0] = (internal_eeprom_read(address) & 0x7f);

    for (uint8_t i = 1; i < chunks[chunkID].size; i++) {
        destination[i] = (internal_eeprom_read(address + i));
    }
}

void read_chunk(uint8_t chunkID, uint8_t *destination) {
    LOG_TRACE({ println("read_chunk"); });

    int16_t address = find_current_record(chunkID);

    if (address != NO_VALID_RECORD) {
        LOG_INFO({ printf("found valid records at: %d\r\n", address); });

        read_record(address, chunkID, destination);
    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

/* -------------------------------------------------------------------------- */

void write_record(uint16_t address, uint8_t chunkID, uint8_t *source) {
    internal_eeprom_write(address, (source[0] & 0x7f));

    for (uint8_t i = 1; i < chunks[chunkID].size; i++) {
        internal_eeprom_write(address + i, source[i]);
    }
}

void write_chunk(uint8_t chunkID, uint8_t *source) {
    LOG_TRACE({ println("write_chunk"); });

    int16_t address = find_current_record(chunkID);
    if (address == NO_VALID_RECORD) {
        address = chunks[chunkID].start;
    }

    // assemble our records
    uint8_t existingData[MAX_CHUNK_SIZE];
    read_record(address, chunkID, &existingData);

    // make sure what we're trying to save is different
    bool recordsAreDifferent = false;
    for (uint8_t i = 0; i < chunks[chunkID].size; i++) {
        if (existingData[i] != source[i]) {
            recordsAreDifferent = true;
            break;
        }
    }

    if (recordsAreDifferent) {
        // invalidate the old record
        internal_eeprom_write(address, 0xff);

        // increment to the next record location
        address += chunks[chunkID].size;
        if (address > (chunks[chunkID].size * chunks[chunkID].overprovision)) {
            address = chunks[chunkID].start;
        }

        LOG_INFO({ printf("saving records at: %d\r\n", address); });
        write_record(address, chunkID, source);
    }
}