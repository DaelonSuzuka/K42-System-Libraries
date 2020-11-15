#include "settings.h"
#include "os/logging.h"
#include "peripherals/nonvolatile_memory.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

#define MAX_RECORD_SIZE 20
#define MAX_RECORD_SLOTS 20
#define MAX_RECORD_GROUPS 10

typedef struct {
    uint8_t start;
    uint8_t size;
    uint8_t slots;
} settings_group_t;

settings_group_t groups[MAX_RECORD_GROUPS];
uint8_t numOfGroups = 0;

/* ************************************************************************** */

void settings_init(void) {
    for (uint8_t i = 0; i < MAX_RECORD_GROUPS; i++) {
        groups[i].size = 0;
        groups[i].slots = 0;
        groups[i].start = 0;
    }

    log_register();
}

uint8_t settings_create_group(uint8_t size, uint8_t slots) {
    groups[numOfGroups].size = size;
    groups[numOfGroups].slots = slots;

    numOfGroups++;
    return numOfGroups;
}

/* ************************************************************************** */

#define NO_VALID_RECORD -1

/*  find_current_record() steps through EEPROM, reading the first byte of
    every stored record, looking for one where the first bit is 0.
*/
int16_t find_current_record(uint8_t group) {
    uint8_t address = groups[group].start;

    while (address < (groups[group].size * groups[group].slots)) {
        if ((internal_eeprom_read(address) & 0x80) == 0) {
            return address;
        }
        address += groups[group].size;
    }

    // didn't find record
    return NO_VALID_RECORD;
}

/* ************************************************************************** */

void read_record(uint16_t address, uint8_t group, uint8_t *dest) {
    dest[0] = (internal_eeprom_read(address) & 0x7f);

    for (uint8_t i = 1; i < groups[group].size; i++) {
        dest[i] = (internal_eeprom_read(address + i));
    }
}

void settings_load(uint8_t group, uint8_t *data) {
    LOG_TRACE({ println("load_settings"); });

    int16_t address = find_current_record(group);

    if (address != NO_VALID_RECORD) {
        LOG_INFO({ printf("found valid records at: %d\r\n", address); });

        read_record(address, group, data);
    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

/* -------------------------------------------------------------------------- */

void write_record(uint16_t address, uint8_t group, uint8_t *src) {
    internal_eeprom_write(address, (src[0] & 0x7f));

    for (uint8_t i = 1; i < groups[group].size; i++) {
        internal_eeprom_write(address + i, src[i]);
    }
}

void save_settings(uint8_t group, uint8_t *data) {
    LOG_TRACE({ println("save_settings"); });

    int16_t address = find_current_record(group);
    if (address == NO_VALID_RECORD) {
        address = groups[group].start;
    }

    // assemble our records
    uint8_t existingData[MAX_RECORD_SIZE];
    read_record(address, group, &existingData);

    // make sure what we're trying to save is different
    bool recordsAreDifferent = false;
    for (uint8_t i = 0; i < groups[group].size; i++) {
        if (existingData[i] != data[i]) {
            recordsAreDifferent = true;
            break;
        }
    }

    if (recordsAreDifferent) {
        // invalidate the old record
        internal_eeprom_write(address, 0xff);

        // increment to the next record location
        address += groups[group].size;
        if (address > (groups[group].size * groups[group].slots)) {
            address = groups[group].start;
        }

        LOG_INFO({ printf("saving records at: %d\r\n", address); });
        write_record(address, group, data);
    }
}