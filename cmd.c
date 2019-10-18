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

    if (func == NULL) {
        TEL_ERR("%s: reg func is NULL!\n", __FUNCTION__);
        return -EARG;
    }
        
    if (strlen(cmd) > ARG_MAX_LEN) {
        TEL_ERR("%s: cmd %s should not longer than ARG_MAX_LEN!\n", __FUNCTION__, cmd);
        return -EARG;
    }
    
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
    if (i >= SUPPORT_CMD_NO){
	TEL_ERR("%s: %s register failed, only support %d commands!\n", __FUNCTION__, cmd, SUPPORT_CMD_NO);
	return -ECMD_REG_FULL;
    }
    
    return EOK;
}

int cmd_usage(int index, s8 *pcmd, s8 *usage)
{
    if (index < SUPPORT_CMD_NO){

	if (cmd_info[index].command.len > 0){
	    strncpy(pcmd, cmd_info[index].command.cmd, cmd_info[index].command.len);
	    strncpy(usage, cmd_info[index].usage, USAGE_MAX_LEN);
	    return EOK;
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
    return EOK;
}
