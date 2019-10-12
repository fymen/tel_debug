#include "cmd.h"
#include "tel_debug.h"

typedef struct
{
    Command command;
    CallBack func;
    s8 usage[USAGE_MAX_LEN];
}Cmd_info;
static Cmd_info cmd_info[SUPPORT_CMD_NO];

CallBack cmd_find(Command *pcmd)
{
    int i;
    
    for(i=0;i<SUPPORT_CMD_NO;i++){
    	if ((pcmd->len == cmd_info[i].command.len) &&
    	    (!strncmp(pcmd->cmd, cmd_info[i].command.cmd, pcmd->len))){
    	    return cmd_info[i].func;
    	}
    }

    return NULL;
}

int cmd_reg(s8 *cmd, void *func,  s8 *usage)
{
    int i;
    Command tmp_cmd;

    memset(&tmp_cmd, 0, sizeof(tmp_cmd));

    tmp_cmd.len = strlen(cmd);
    strncpy(tmp_cmd.cmd, cmd, tmp_cmd.len);
    for (i=0;i<SUPPORT_CMD_NO;i++){
	if (cmd_info[i].command.len == tmp_cmd.len){
	    if (!strncmp(cmd_info[i].command.cmd, tmp_cmd.cmd, tmp_cmd.len))
		return -ECMD_REG_CONFLICT;
	}
	
	if (cmd_info[i].func == NULL){
	    cmd_info[i].command.len = strlen(cmd);
	    strncpy(cmd_info[i].command.cmd, cmd, cmd_info[i].command.len);
	    cmd_info[i].func = (CallBack)func;
	    if (strlen(usage) < USAGE_MAX_LEN)
		strncpy(cmd_info[i].usage, usage, strlen(usage));
	    else
		strncpy(cmd_info[i].usage, usage, USAGE_MAX_LEN-1);
	    break;
	}
    }
    if (i >= ARG_MAX_NO){
	TEL_ERR("%s: too many args..\n", __FUNCTION__);
	return -ECMD_REG_FULL;
    }
    
    return 0;
}

int cmd_usage(int index, s8 *pcmd, s8 *usage)
{
    if (index < SUPPORT_CMD_NO){

	if (cmd_info[index].command.len > 0){
	    strncpy(pcmd, cmd_info[index].command.cmd, cmd_info[index].command.len);
	    strncpy(usage, cmd_info[index].usage, USAGE_MAX_LEN);
	    return 0;
	}
    }
    return -ECMD_NOT_EXIST;
}

int cmd_init(void)
{
    memset(cmd_info, 0, sizeof(cmd_info));
    return EOK;
}
int cmd_exit(void)
{
    return 0;
}
