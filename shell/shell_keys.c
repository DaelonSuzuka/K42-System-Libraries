#include "shell_keys.h"
#include "serial_port.h"
#include "shell_config.h"
#include "system_time.h"
#include <stdbool.h>
#include <string.h>

/* ************************************************************************** */

#define X(NAME) #NAME,
const char *keyNameString[] = {KEY_NAME_LIST};
const char *keyModifierString[] = {KEY_MODIFIER_LIST};
#undef X

void print_key(key_t *key) {
    if (key->mod != NONE) {
        print(keyModifierString[key->mod]);
        print(" + ");
    }
    print(keyNameString[key->key]);
}

/* ************************************************************************** */
/*  sequence_t is a data type that stores an ASCII control/escape sequence.

    The default value of SEQUENCE_BUFFER_LENGTH is 10, but the longest control
    sequence I know of is only 6 characters long.
*/
typedef struct {
    char buffer[SEQUENCE_BUFFER_LENGTH];
    uint8_t length;
} sequence_t;

sequence_t new_sequence(void) {
    sequence_t sequence;
    memset(&sequence, 0, sizeof(sequence));
    return sequence;
}

void print_sequence(sequence_t sequence) {
    print("{");
    for (uint8_t i = 0; i < sequence.length; i++) {
        putch(sequence.buffer[i]);
    }

    print("}");
}

/* -------------------------------------------------------------------------- */

typedef enum {
    KEY_UP = 'A',
    KEY_DOWN = 'B',
    KEY_RIGHT = 'C',
    KEY_LEFT = 'D',
    KEY_HOME = 'H',
    KEY_HOME2 = '1',
    KEY_END = 'F',
    KEY_END2 = '4',
    KEY_DEL = '3',
    KEY_INS = '2',
    KEY_PGUP = '5',
    KEY_PGDN = '6',
    KEY_F1 = 'P',
    KEY_F2 = 'Q',
    KEY_F3 = 'R',
    KEY_F4 = 'S',
    KEY_F5 = '6',
    KEY_F5_ALT = '5',
    KEY_F6 = '7',
    KEY_F7 = '8',
    KEY_F8 = '9',
    KEY_F9 = '0',
    KEY_F10 = '1',
    KEY_F11 = '3',
    KEY_F12 = '4',
} sequenceIdentifiers;

/*  decode_escape_sequence()

    Escape sequences can be uniquely identified by knowing the length of the
    sequence and one or two characters from the sequence. Escape sequences can
    be very different when using different terminal emulators. This decoder was
    built with a mix of VT102 and xterm, but there's no guarantees that escape
    sequences will be identical with other machines or configurations.

    Example sequences:
    up arrow: (ESC, 65)
    pageup: (ESC, 53, ~)
*/
static key_t decode_escape_sequence(sequence_t *sequence) {
    key_t newKey = {UNKNOWN, NONE};

    switch (sequence->length) {
    case 0:
        newKey.key = ESCAPE;
        return newKey;
    case 2:
        switch (sequence->buffer[1]) {
        default:
            return newKey;
        case KEY_UP:
            newKey.key = UP;
            return newKey;
        case KEY_DOWN:
            newKey.key = DOWN;
            return newKey;
        case KEY_RIGHT:
            newKey.key = RIGHT;
            return newKey;
        case KEY_LEFT:
            newKey.key = LEFT;
            return newKey;
        case KEY_HOME:
            newKey.key = HOME;
            return newKey;
        case KEY_END:
            newKey.key = END;
            return newKey;
        case KEY_F1:
            newKey.key = F1;
            return newKey;
        case KEY_F2:
            newKey.key = F2;
            return newKey;
        case KEY_F3:
            newKey.key = F3;
            return newKey;
        case KEY_F4:
            newKey.key = F4;
            return newKey;
        }
    case 3:
        if (sequence->buffer[2] == '~') {
            switch (sequence->buffer[1]) {
            default:
                return newKey;
            case KEY_HOME2:
                newKey.key = HOME;
                return newKey;
            case KEY_END2:
                newKey.key = END;
                return newKey;
            case KEY_PGUP:
                newKey.key = PAGEUP;
                return newKey;
            case KEY_PGDN:
                newKey.key = PAGEDOWN;
                return newKey;
            case KEY_DEL:
                newKey.key = DELETE;
                return newKey;
            case KEY_INS:
                newKey.key = INSERT;
                return newKey;
            }
        }
        return newKey;
    case 4:
        if (sequence->buffer[3] == '~') {
            switch (sequence->buffer[2]) {
            default:
                return newKey;
            case KEY_F5:
            case KEY_F5_ALT:
                newKey.key = F5;
                return newKey;
            case KEY_F6:
                newKey.key = F6;
                return newKey;
            case KEY_F7:
                newKey.key = F7;
                return newKey;
            case KEY_F8:
                newKey.key = F8;
                return newKey;
            case KEY_F9:
                newKey.key = F9;
                return newKey;
            case KEY_F10:
                newKey.key = F10;
                return newKey;
            case KEY_F11:
                newKey.key = F11;
                return newKey;
            case KEY_F12:
                newKey.key = F12;
                return newKey;
            }
        }
        return newKey;
    case 5:
        switch (sequence->buffer[4]) {
        default:
            return newKey;
        case KEY_F1:
            newKey.key = F1;
            break;
        case KEY_F2:
            newKey.key = F2;
            break;
        case KEY_F3:
            newKey.key = F3;
            break;
        case KEY_F4:
            newKey.key = F4;
            break;
        case KEY_UP:
            newKey.key = UP;
            break;
        case KEY_DOWN:
            newKey.key = DOWN;
            break;
        case KEY_RIGHT:
            newKey.key = RIGHT;
            break;
        case KEY_LEFT:
            newKey.key = LEFT;
            break;
        case KEY_HOME:
            newKey.key = HOME;
            break;
        case KEY_END:
            newKey.key = END;
            break;
        case '~':
            if (sequence->buffer[1] == KEY_DEL) {
                newKey.key = DELETE;
                break;
            }
            if (sequence->buffer[1] == KEY_INS) {
                newKey.key = INSERT;
                break;
            }
        }
        newKey.mod = sequence->buffer[3] - '0';
        return newKey;
    case 6:
        if (sequence->buffer[5] == '~') {
            switch (sequence->buffer[2]) {
            default:
                return newKey;
            case KEY_F5:
            case KEY_F5_ALT:
                newKey.key = F5;
                break;
            case KEY_F6:
                newKey.key = F6;
                break;
            case KEY_F7:
                newKey.key = F7;
                break;
            case KEY_F8:
                newKey.key = F8;
                break;
            case KEY_F9:
                newKey.key = F9;
                break;
            case KEY_F10:
                newKey.key = F10;
                break;
            case KEY_F11:
                newKey.key = F11;
                break;
            case KEY_F12:
                newKey.key = F12;
                break;
            }
            newKey.mod = sequence->buffer[4] - '0';
            return newKey;
        }
        return newKey;
    default:
        return newKey;
    }
}

/* -------------------------------------------------------------------------- */
/*  intercept_escape_sequence() should only be called when we're recieved an
    ESC character and are expecting an unknown number of additional characters.

    Since we don't know how many characters are coming, we just have to call
    getch() repeatedly. Every time we recieve a character, run the sequence
    throught decode_escape_sequence(). If that returns a valid key, then quit
    early. If it never matches, quit after ~5mS.
*/
static sequence_t intercept_escape_sequence(void) {
    sequence_t sequence = new_sequence();

    system_time_t startTime = get_current_time();
    while (time_since(startTime) < 5) {
        // check for a new character
        sequence.buffer[sequence.length] = getch();
        // if valid character, move to next spot in buffer
        if (sequence.buffer[sequence.length] != 0) {
            sequence.length++;

            key_t key = decode_escape_sequence(&sequence);
            if (key.key != UNKNOWN) {
                break;
            }
        }
    }

    return sequence;
}

/* -------------------------------------------------------------------------- */

typedef enum {
    KEY_ETX = 3,
    KEY_CTRL_D = 4,
    KEY_CTRL_E = 5,
    KEY_BS = 8,
    KEY_TAB = 9,
    KEY_LF = 10,
    KEY_CR = 13,
    KEY_CTRL_U = 21,
    KEY_CTRL_Y = 25,
    KEY_ESC = 27,
    KEY_CTRL_Z = 26,
    KEY_CTRL_BS = 31,
} controlCharacters;

/*  decode_control_character() identifies a key from a single control character.

    This only identifies a subset of the non-sequence control characters.
*/
static key_t decode_control_character(char currentChar) {
    key_t newKey = {UNKNOWN, NONE};

    switch (currentChar) {
    default:
        return newKey;
    case KEY_BS:
        newKey.key = BACKSPACE;
        return newKey;
    case KEY_LF:
    case KEY_CR:
        newKey.key = ENTER;
        return newKey;
    case KEY_TAB:
        newKey.key = TAB;
        return newKey;
    }
}

/* ************************************************************************** */

static bool diagnosticsEnabled = false;

void toggle_key_diagnostics(void) {

    diagnosticsEnabled = !diagnosticsEnabled;
    if (diagnosticsEnabled) {
        println("\r\nEscape sequence diagnostics enabled.");
    } else {
        println("\r\nEscape sequence diagnostics disabled.");
    }
}

// returns a key object that identifies the pressed key
key_t identify_key(char currentChar) {
    sequence_t sequence = new_sequence();
    key_t key = {UNKNOWN, NONE};

    if (currentChar == KEY_ESC) {
        sequence = intercept_escape_sequence();
        key = decode_escape_sequence(&sequence);
    } else {
        key = decode_control_character(currentChar);
    }

    if (diagnosticsEnabled) {
        if (currentChar == KEY_ESC) {
            print_sequence(sequence);
        } else {
            printf("{%d} ", currentChar);
        }
        print(" ");
        print_key(&key);
        println("");
    }

    return key;
}