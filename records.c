#include "records.h"
#include "os/logging.h"
#include "peripherals/nonvolatile_memory.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

#define MAX_RECORD_SIZE 20
#define MAXIMUM_NUMBER_OF_RECORDS 10

typedef struct {
    uint16_t start;
    uint16_t size;
    uint16_t overprovision;
    uint16_t length;
} record_metadata_t;

record_metadata_t records[MAXIMUM_NUMBER_OF_RECORDS];
uint8_t numOfRecords = 0;

void _print_record(uint8_t recordID) {
    printf("ID: %d start: %3d, size: %3d, length: %3d op: %3d \r\n", recordID,
           records[recordID].start, records[recordID].size,
           records[recordID].length,
           records[recordID].overprovision);
}

void _print_records(void) {
    for (uint8_t i = 0; i < numOfRecords; i++) {
        _print_record(i);
    }
    printf("\r\n");
}

/* ************************************************************************** */

void records_init(uint16_t minAddress, uint16_t maxAddress) {
    // clear all the chunk data
    for (uint8_t i = 0; i < MAXIMUM_NUMBER_OF_RECORDS; i++) {
        records[i].size = 0;
        records[i].overprovision = 0;
        records[i].start = 0;
    }

    // make the first chunk start at the minimum allowed address
    records[0].start = minAddress;

    // TODO: implement maxAddress

    log_register();
}

uint8_t create_new_record(uint16_t recordSize, uint16_t overprovisionFactor,
                          enum recordLocation location) {
    records[numOfRecords].size = recordSize;
    records[numOfRecords].overprovision = overprovisionFactor;

    uint16_t nextStart = records[numOfRecords].start + 1;

    if (overprovisionFactor) {
        records[numOfRecords].length = (recordSize + 1) * overprovisionFactor;
    } else {
        records[numOfRecords].length = recordSize;
    }
    nextStart += records[numOfRecords].length;

    numOfRecords++;
    records[numOfRecords].start = nextStart;

    return numOfRecords - 1;
}

/* ************************************************************************** */

#define NO_VALID_RECORD -1

int16_t get_record_address(uint8_t recordID) {
    if (records[recordID].overprovision == 0) {
        return records[recordID].start;
    }

    uint16_t address = records[recordID].start;
    uint16_t end = address + records[recordID].length;

    while (address < end) {
        // printf("checking: %d (%d) \r\n", address,
        //        internal_eeprom_read(address));
        if (internal_eeprom_read(address) == 0) {

            // println("found");
            return address;
        }
        address += records[recordID].size + 1;
    }

    // didn't find record
    // println("didn't find");
    return records[recordID].start;
}

/* ************************************************************************** */

void read_slice(uint16_t address, uint8_t recordID, uint8_t *destination) {
    if (records[recordID].overprovision) {
        address += 1;
    }

    // printf("reading from %d\r\n", address);

    for (uint8_t i = 0; i < records[recordID].size; i++) {
        destination[i] = (internal_eeprom_read(address + i));
    }
}

void load_record(uint8_t recordID, uint8_t *destination) {
    LOG_TRACE({ println("load_record"); });

    int16_t address = get_record_address(recordID);

    if (address != NO_VALID_RECORD) {
        if (records[recordID].overprovision) {
            // address += 1;
        }
        LOG_INFO({ printf("found valid record at: %d\r\n", address); });
        read_slice(address, recordID, destination);
    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

/* -------------------------------------------------------------------------- */

void write_slice(uint16_t address, uint8_t recordID, uint8_t *source) {
    if (records[recordID].overprovision) {
        address += 1;
    }
    // printf("writing to %d\r\n", address);

    for (uint8_t i = 0; i < records[recordID].size; i++) {
        internal_eeprom_write(address + i, source[i]);
    }
}

void store_record(uint8_t recordID, uint8_t *source) {
    LOG_TRACE({ println("store_record"); });

    int16_t address = get_record_address(recordID);
    if (address == NO_VALID_RECORD) {
        address = records[recordID].start;
    }

    // assemble our records
    uint8_t existingData[MAX_RECORD_SIZE];
    read_slice(address, recordID, &existingData);

    // make sure what we're trying to save is different
    bool recordsAreDifferent = false;
    for (uint8_t i = 0; i < records[recordID].size; i++) {
        if (existingData[i] != source[i]) {
            recordsAreDifferent = true;
            break;
        }
    }

    if (recordsAreDifferent) {
        // invalidate the old record
        internal_eeprom_write(address, 0xff);

        // increment to the next record location
        address += records[recordID].size + 1;

        uint16_t end = records[recordID].start + records[recordID].length;
        if (address > end)  {
            address = records[recordID].start;
        }

        if (records[recordID].overprovision) {
            internal_eeprom_write(address, 0);
        }
        LOG_INFO({ printf("saving record at: %d\r\n", address); });
        write_slice(address, recordID, source);
    }
}

/* ************************************************************************** */

#include "os/shell/shell.h"
#include "os/shell/shell_command_utils.h"
#include "os/shell/shell_keys.h"
#include "os/shell/shell_utils.h"

/* ************************************************************************** */

static void sh_records(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage:\trecords list");
        println("\trecords create <size> <opFactor>");
        println("\trecords load <recordID>");
        println("\trecords store <recordID> <data>");
        return;

    case 2:
        if (!strcmp(argv[1], "list")) {
            _print_records();
            return;
        }
        break;

    case 3:
        if (!strcmp(argv[1], "load")) {
            uint16_t recordID = atoi(argv[2]);
            uint8_t loadBuffer[50] = {};

            load_record(recordID, &loadBuffer);
            println(loadBuffer);

            return;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "create")) {
            uint16_t size = atoi(argv[2]);
            uint16_t opFactor = atoi(argv[3]);

            uint8_t recordID = create_new_record(size, opFactor, record_eeprom);

            printf("new record ID: %d\r\n", recordID);
            return;

        } else if (!strcmp(argv[1], "store")) {
            uint16_t recordID = atoi(argv[2]);

            println(argv[3]);

            // uint8_t data = atoi(argv[3]);
            // uint8_t storeBuffer[50] = {};

            store_record(recordID, argv[3]);

            return;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return;
}

REGISTER_SHELL_COMMAND(sh_records, "records");