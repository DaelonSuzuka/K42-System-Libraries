#include "os/json/json_messages.h"
#include "os/json/json_print.h"
#include "os/libs/str_len.h"
#include "os/system_time.h"
#include "shell_command_processor.h"
#include "shell_command_utils.h"
#include "shell_config.h"
#include "shell_keys.h"
#include "system.h"
#include <string.h>

/* ************************************************************************** */

// prints all registered commands
void shell_help(int argc, char **argv) {
    println("-----------------------------------------------");
    print_command_list();
    println("-----------------------------------------------");
}

REGISTER_SHELL_COMMAND(shell_help, "help");

/* -------------------------------------------------------------------------- */

system_time_t rebootStartTime;

int8_t reboot_callback(char currentChar) {
    if (time_since(rebootStartTime) > 5000) {
        println("aborting");
        delay_ms(250);
        return -1;
    }

    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        switch (key.key) {
        default:
            return 0;

        case ESCAPE:
            return -1;
        }
    }

    if (isprint(currentChar)) {
        printf("%c\r\n", currentChar);

        if (currentChar == 'y' || currentChar == 'Y') {
            println("rebooting");
            delay_ms(50);
            asm("RESET");
            return 0;
        }
        return -1;
    }

    return 0;
}

//
void shell_reboot(int argc, char **argv) {
    if ((argc == 2) && (!strcmp(argv[1], "-y"))) {
        println("rebooting");
        asm("RESET");
        return;
    }

    print("confirm reboot [Y/n]: ");

    shell_register_callback(reboot_callback);
    rebootStartTime = get_current_time();
}

REGISTER_SHELL_COMMAND(shell_reboot, "reboot");

/* -------------------------------------------------------------------------- */

void shell_arg_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        println("This command has no special arguments.");
        println("It is designed to test the TuneOS shell's arg parsing.");
        println("Use it like this:");
        println("\"$ test command arg1 arg2 arg3\"");
        println("");
        println("To get this response:");
        println("Received 4 arguments for test command");
        println("1 - \"command\" [len:7]");
        println("2 - \"arg1\" [len:4]");
        println("3 - \"arg2\" [len:4]");
        println("4 - \"arg3\" [len:4]");
    } else {
        printf("Received %d arguments for test command\r\n", argc - 1);

        // Prints: <argNum> - "<string>" [len:<length>]
        for (uint8_t i = 1; i < argc; i++) {
            // printf("%u - \"%s\" [len:%u]\r\n", i, argv[i], str_len(argv[i]));
            printf("%u - \"", i);
            print(argv[i]);
            printf("\" [len:%u]\r\n", str_len(argv[i]));
        }
    }
    println("-----------------------------------------------");
}

REGISTER_SHELL_COMMAND(shell_arg_test, "test");

/* -------------------------------------------------------------------------- */

// JSON object definitions

void shell_version(int argc, char **argv) {
    if ((argc == 2) && (!strcmp(argv[1], "-j"))) {
        json_print(print, deviceInfo);
        println("");
        return;
    }

    printf("%s %s", &productName, &productVersion);
    println("");
    printf("Chitin v%s", SHELL_VERSION);
    println("");
    printf("Compiled on %s at %s", __DATE__, __TIME__);
    printf(" using XC8 v%u", __XC8_VERSION);
    println("");
}

REGISTER_SHELL_COMMAND(shell_version, "version");