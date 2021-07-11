#ifndef _SHELL_COLORS_H_
#define _SHELL_COLORS_H_

/* ************************************************************************** */
// ANSI sequence text formatting helpers

// effects, apply after another color
#define TXT_BOLD "\033[1m"
#define TXT_UNDERLINE "\033[4m"

// base colors
#define TXT_BLACK "\033[0;30m"
#define TXT_RED "\033[0;31m"
#define TXT_GREEN "\033[0;32m"
#define TXT_YELLOW "\033[0;33m"
#define TXT_BLUE "\033[0;34m"
#define TXT_MAGENTA "\033[0;35m"
#define TXT_CYAN "\033[0;36m"
#define TXT_WHITE "\033[0;37m"

// bright colors
#define TXT_BR_BLACK "\033[0;90m"
#define TXT_BR_RED "\033[0;91m"
#define TXT_BR_GREEN "\033[0;92m"
#define TXT_BR_YELLOW "\033[0;93m"
#define TXT_BR_BLUE "\033[0;94m"
#define TXT_BR_MAGENTA "\033[0;95m"
#define TXT_BR_CYAN "\033[0;96m"
#define TXT_BR_WHITE "\033[0;97m"

// base colors, bold
#define TXT_BOLD_BLACK "\033[1;30m"
#define TXT_BOLD_RED "\033[1;31m"
#define TXT_BOLD_GREEN "\033[1;32m"
#define TXT_BOLD_YELLOW "\033[1;33m"
#define TXT_BOLD_BLUE "\033[1;34m"
#define TXT_BOLD_MAGENTA "\033[1;35m"
#define TXT_BOLD_CYAN "\033[1;36m"
#define TXT_BOLD_WHITE "\033[1;37m"

// background colors
#define TXT_BG_BLACK "\033[40m"
#define TXT_BG_RED "\033[41m"
#define TXT_BG_GREEN "\033[42m"
#define TXT_BG_YELLOW "\033[43m"
#define TXT_BG_BLUE "\033[44m"
#define TXT_BG_MAGENTA "\033[45m"
#define TXT_BG_CYAN "\033[46m"
#define TXT_BG_WHITE "\033[47m"

// background colors, bright
#define TXT_BG_BR_BLACK "\033[100m"
#define TXT_BG_BR_RED "\033[101m"
#define TXT_BG_BR_GREEN "\033[102m"
#define TXT_BG_BR_YELLOW "\033[103m"
#define TXT_BG_BR_BLUE "\033[104m"
#define TXT_BG_BR_MAGENTA "\033[105m"
#define TXT_BG_BR_CYAN "\033[106m"
#define TXT_BG_BR_WHITE "\033[107m"

#define TXT_RESET "\033[0;37;40m"

#endif // _SHELL_COLORS_H_