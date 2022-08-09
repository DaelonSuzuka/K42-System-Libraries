#ifndef _SHELL_BUILTINS_H_
#define _SHELL_BUILTINS_H_

/* ************************************************************************** */

extern void sh_help(int argc, char **argv);
extern void sh_clear(int argc, char **argv);
extern void sh_reboot(int argc, char **argv);
extern void sh_test(int argc, char **argv);
extern void sh_version(int argc, char **argv);
extern void sh_colors(int argc, char **argv);

#endif // _SHELL_BUILTINS_H_