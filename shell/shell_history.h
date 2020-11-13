#ifndef _SHELL_HISTORY_H_
#define _SHELL_HISTORY_H_

/* ************************************************************************** */

// setup
extern void shell_history_init(void);

// wipe out the whole history and start over
extern void shell_history_wipe(void);

// 
extern void toggle_history_inspection_mode(void);

/* -------------------------------------------------------------------------- */

// push the current line onto the history stack
extern void shell_history_push(void);

// step to the next oldest history
extern void shell_history_show_older(void);

// step to the next newest history, eventually returning to the current line
extern void shell_history_show_newer(void);

#endif