#ifndef CMD_H
#define CMD_H
#include "common.h"

#define ARG_MAX_LEN 32
#define ARG_MAX_NO 5
#define CMD_LINE_MAX_LEN ((ARG_MAX_LEN) * ((ARG_MAX_NO) + 1))
#define USAGE_MAX_LEN 128
#define SUPPORT_CMD_NO 32

typedef struct Command
{
    s8 cmd[ARG_MAX_LEN];
    s8 len;
}Command;
typedef int (*CallBack)(int arg, int arg1, int arg2, int arg3);

CallBack cmd_find(Command *pcmd);
int cmd_reg(s8 *cmd, void *func,  s8 *usage);
int cmd_usage(int index, s8 *pcmd, s8 *usage);
int cmd_init(void);
int cmd_exit(void);

#endif
 
