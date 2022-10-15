#include "os/libs/str_len.h"
#include "os/serial_port.h"
#include "os/system_time.h"
#include "shell_command_utils.h"
#include "shell_config.h"
#include "shell_cursor.h"
#include "system.h"

/* ************************************************************************** */

// prints all registered commands
void sh_help(int argc, char **argv) {
    sh_println("-----------------------------------------------");
    print_command_list();
    sh_println("-----------------------------------------------");
}

/* -------------------------------------------------------------------------- */

void sh_clear(int argc, char **argv) {
    term_reset_screen();
    term_cursor_set(0, 0);
}

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
void sh_reboot(int argc, char **argv) {
    if ((argc == 2) && (!strcmp(argv[1], "-y"))) {
        println("rebooting");
        asm("RESET");
        return;
    }

    print("confirm reboot [Y/n]: ");

    shell_register_callback(reboot_callback);
    rebootStartTime = get_current_time();
}

/* -------------------------------------------------------------------------- */

void sh_test(int argc, char **argv) {
    sh_println("-----------------------------------------------");
    sh_println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        sh_println("This command has no special arguments.");
        sh_println("It is designed to test chitin's arg parsing.");
        sh_println("Use it like this:");
        sh_println("\"$ test command arg1 arg2 arg3\"");
        sh_println("");
        sh_println("To get this response:");
        sh_println("Received 4 arguments for test command");
        sh_println("1 - \"command\" [len:7]");
        sh_println("2 - \"arg1\" [len:4]");
        sh_println("3 - \"arg2\" [len:4]");
        sh_println("4 - \"arg3\" [len:4]");
    } else {
        printf("Received %d arguments for test command\r\n", argc - 1);

        // Prints: <argNum> - "<string>" [len:<length>]
        for (uint8_t i = 1; i < argc; i++) {
            // printf("%u - \"%s\" [len:%u]\r\n", i, argv[i], str_len(argv[i]));
            printf("%u - \"", i);
            sh_print(argv[i]);
            printf("\" [len:%u]\r\n", strlen(argv[i]));
        }
    }
    sh_println("-----------------------------------------------");
}

/* -------------------------------------------------------------------------- */

void sh_version(int argc, char **argv) {
    printf("%s %s", &productName, &productVersion);
    println("");
    printf("%s", &processorModel);
    println("");
    printf("Chitin v%s", SHELL_VERSION);
    println("");
    printf("Compiled on %s at %s", &compileDate, &compileTime);
    printf(" using XC8 v%u", __XC8_VERSION);
    println("");
}

/* -------------------------------------------------------------------------- */

// displays a color rendering demo
void sh_colors(int argc, char **argv) {
    sh_println("-----------------------------------------------");

    println(TXT_BLACK "BLACK "     //
            TXT_RED "RED "         //
            TXT_GREEN "GREEN "     //
            TXT_YELLOW "YELLOW "   //
            TXT_BLUE "BLUE "       //
            TXT_MAGENTA "MAGENTA " //
            TXT_CYAN "CYAN "       //
            TXT_WHITE "WHITE "     //
            TXT_RESET "| BASE"     //
    );

    println(TXT_BR_BLACK "BLACK "     //
            TXT_BR_RED "RED "         //
            TXT_BR_GREEN "GREEN "     //
            TXT_BR_YELLOW "YELLOW "   //
            TXT_BR_BLUE "BLUE "       //
            TXT_BR_MAGENTA "MAGENTA " //
            TXT_BR_CYAN "CYAN "       //
            TXT_BR_WHITE "WHITE "     //
            TXT_RESET "| BRIGHT"      //
    );

    println(TXT_BOLD_BLACK "BLACK "     //
            TXT_BOLD_RED "RED "         //
            TXT_BOLD_GREEN "GREEN "     //
            TXT_BOLD_YELLOW "YELLOW "   //
            TXT_BOLD_BLUE "BLUE "       //
            TXT_BOLD_MAGENTA "MAGENTA " //
            TXT_BOLD_CYAN "CYAN "       //
            TXT_BOLD_WHITE "WHITE "     //
            TXT_RESET "| BOLD"          //
    );

    println(TXT_BG_BLACK "BLACK "               //
            TXT_BLACK TXT_BG_RED "RED "         //
            TXT_BLACK TXT_BG_GREEN "GREEN "     //
            TXT_BLACK TXT_BG_YELLOW "YELLOW "   //
            TXT_BLACK TXT_BG_BLUE "BLUE "       //
            TXT_BLACK TXT_BG_MAGENTA "MAGENTA " //
            TXT_BLACK TXT_BG_CYAN "CYAN "       //
            TXT_BLACK TXT_BG_WHITE "WHITE "     //
            TXT_RESET "| BACKGROUNDS"           //
    );

    println(TXT_BLACK TXT_BG_BR_BLACK "BLACK "     //
            TXT_BLACK TXT_BG_BR_RED "RED "         //
            TXT_BLACK TXT_BG_BR_GREEN "GREEN "     //
            TXT_BLACK TXT_BG_BR_YELLOW "YELLOW "   //
            TXT_BLACK TXT_BG_BR_BLUE "BLUE "       //
            TXT_BLACK TXT_BG_BR_MAGENTA "MAGENTA " //
            TXT_BLACK TXT_BG_BR_CYAN "CYAN "       //
            TXT_BLACK TXT_BG_BR_WHITE "WHITE "     //
            TXT_RESET "| BACKGROUNDS, BRIGHT"      //
    );

    sh_println("-----------------------------------------------");
}