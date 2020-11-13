#include "str_len.h"

/* ************************************************************************** */

uint8_t str_len(const char *string) {
    const char *cp = string;

    while (*cp) {
        cp++;
    }

    return cp - string;
}


/*  
size_t strlen_C90(const char *s) {
    register const char *cp;

    cp = s;
    while (*cp) {
        cp++;
    }
    return cp - s;
}

size_t strlen_C99(const char *s) {
    const char *a = s;
    while (*s) {
        s++;
    }
    return s - a;
}

static uint8_t string_length(char *s) {
    char *cp = s;
    while (*cp) {
        cp++;
    }
    return cp - s;
}

void test2(char* string){

}

void test(void) {
    // fake shell input
    char buffer[] = "test 1 2 3 4";
    print("buffer(before strlen): ");
    println(buffer);
    printf("string_length: %u\r\n", string_length(buffer));
    printf("strlen: %u\r\n", strlen(buffer));
    print("buffer(after strlen): ");
    println(buffer);

    // shell arguments
    int argc = 0;
    char *argv[10] = {0};

    println("tokenizing the buffer...");
    char *token = strtok(buffer, " ");
    while (token != NULL && argc < 10) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    println("");
    println(buffer);
    printf("argc: %u\r\n", argc);

    println("");
    println("printing argv contents");
    for (uint8_t i = 0; i < argc; i++) {
        println(argv[i]);
    }

    println("");
    println("arg parsing test loop with string_length");
    for (uint8_t i = 0; i < argc; i++) {
        printf("%u - \"%s\" [len:%u]\r\n", i, argv[i], string_length(argv[i]));
    }

    println("");
    println("arg parsing test loop with strlen");
    for (uint8_t i = 0; i < argc; i++) {
        printf("%u - \"%s\" [len:%u]\r\n", i, argv[i], strlen(argv[i]));
    }
}
*/