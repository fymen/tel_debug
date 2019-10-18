#include "common.h"
#include <net/if.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "net.h"
#include <math.h>
#include "cmd.h"
#include "tel_debug.h"


static s32 gclient_sock;
static s32 gserver_sock;
typedef struct thread_data
{
    pthread_t telnetd_handle;
    s32 telnetd_enable;
}Thread_data;
static Thread_data thread_data;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void tel_print(char* fmt, ...);

static int print_char(s8 c)
{
    return net_send(gclient_sock,(u8 *)&c, 1);
}
static int print_str(s8 *s)
{
    int ret = 0;
    
    while(*s){
	ret |= print_char(*s++);
    }
    return ret;
}
static void print_hex(unsigned int hex)
{
    int tmp, i = 0;
    int hex_value;
    s8 buf[32] = {0};
    hex_value = hex;
    do{
	tmp = hex_value % 16;
	if (tmp < 10)
	    buf[i++] = tmp + '0';
	else
	    buf[i++] = tmp - 10 + 'A';
	hex_value = hex_value / 16;
    }while(hex_value != 0);

    while (i-- > 0){
	print_char(buf[i]);
    }
}

void print_dec(s32 dec)
{
    int tmp, i = 0;
    s8 buf[32] = {0};

    if (dec < 0){
	print_char('-');
	tmp = -dec;
    }
    else
	tmp = dec;

    do{
	buf[i++] = (tmp % 10) + '0';
	tmp = tmp / 10;
    }while(tmp != 0);

    while (i-- > 0){
	print_char(buf[i]);
    }
}
static void print_float(double f)
{
    int tmp;
    double test;
    
    tmp = (int)f;
    print_dec(tmp);
    test = f - tmp;

    tmp = (int)(test * 100000000 + 0.1);
    print_char('.');
    print_dec(tmp);
}

void prompt_login(void)
{
    tel_print("###########################################\n");
    tel_print("CAUTION:\n");
    tel_print("1.support up to %d commands\n", SUPPORT_CMD_NO-2);
    tel_print("2.a single command or arg is limited to %d bytes\n", ARG_MAX_LEN);
    tel_print("3.support up to %d args\n", ARG_MAX_NO);
    tel_print("4.the buffer for usage discribtion is limited to %d bytes\n", USAGE_MAX_LEN);
    tel_print("5.print \"help\" to get a help list\n");
    tel_print("###########################################\n");
}

s8 *swallow_space(s8 *s)
{
    s8 *p;
    p = s;
    while (*p == ' ')
    	p++;

    return p;
}

/* support digital number as the args only */
int parse_cmd(s8 *str)
{
    s8 *p;
    Command tmp_cmd;
    s8 arg[ARG_MAX_NO][ARG_MAX_LEN];
    s32 darg[ARG_MAX_NO] = {0};
    s32 i, j;

    int ret = 0;

    memset(&tmp_cmd, 0,sizeof(tmp_cmd));
    memset(&arg, 0, sizeof(arg));
    
    i = j = 0;
    p = str;
    p = swallow_space(p);
    /* extract the command */
    while ((*p != '\0') && (*p != ' ')){
	if (j < ARG_MAX_LEN)
	    tmp_cmd.cmd[j++] = *p++;
	else{
	    tel_print("excced lenghth of args\n");
	    p++;
	}
    }
    tmp_cmd.len = j;
    j = 0;
    p = swallow_space(p);
    
    /* extract the args */
    while ((*p != '\0') && (i < ARG_MAX_NO)){
	if (*p == ' '){
	    p = swallow_space(p);
	    if (*p == '\0')
		break;
	    i++;
	    j = 0;
	    continue;
	}
	if (j < ARG_MAX_LEN)
	    arg[i][j++] = *p++;
	else{
	    tel_print("excced lenghth of args\n");
	    p++;
	}
	
    }

    if (i >= (ARG_MAX_NO)) {
        tel_print("command line \"%s\" has arguments more than %d\n", str, ARG_MAX_NO);

        return -EARG;
    }

    /* transform the args to digital format */
    for (j=0;j<=i;j++){
	if ((arg[j][1] == 'x') || (arg[j][1] == 'X')) /* hex number */
	    darg[j] = strtol(arg[j], NULL, 0);
	else
	    darg[j] = atoi(arg[j]);
    }

    /* call function */
    CallBack func;
    func = cmd_find(&tmp_cmd);
    if (!func){
	tel_print("Do not support command line: %s\n", str);
	return -EARG;
    }
    ret = func(darg[0], darg[1], darg[2], darg[3]);
    tel_print("\nret=%d\n", ret);

    return EOK;
}

/* Telnet Server recvive thread */
static void telnetd_rcv(void)
{
    u8 cmd_char;
    s8 cmd_line[CMD_LINE_MAX_LEN] = {0};
    s32 i = 0;
    s32 ret;

    pthread_detach(pthread_self());
    while(1){
	if (thread_data.telnetd_enable == FALSE)
	    break;
	
	if (gclient_sock <= 0){
	    TEL_PRT("gclient_sock invaild.. %d\n", gclient_sock);
	    usleep(100*1000);

	    gclient_sock = net_accept(gserver_sock);

	    prompt_login();
	    tel_print(">>");
	    continue;
	}
	
	ret = net_recv1char(gclient_sock, &cmd_char);
	if (1 != ret ){
	    TEL_PRT("%s:recv error %d\n", __FUNCTION__, ret);
	    gclient_sock = -1;
	    continue;
	}

	switch(cmd_char) {
	case 0:
	    tel_print(">>");
	    break;
	case 0xEF:
	    break;
	case CTRL_S:
	    break;
	case TAB_CHAR:
	    break;
	case 0x7f:
	    tel_print("\b \b");
	    break;
	case RETURN_CHAR:
	    /* tel_print("\n"); */
	    if (i > 0){
		parse_cmd(cmd_line);
		memset(cmd_line, '\0', sizeof(cmd_line));
		i = 0;
	    }
	    break;
	case NEWLINE_CHAR:
	    tel_print(">>");
	    fsync(gclient_sock);
	    break;
	default:
	    /* tel_print("%c", cmd_char); /\* echo back to client *\/ */
	    if ((cmd_char >= 0x20) && (cmd_char <= 0x7E) &&
		(i < (CMD_LINE_MAX_LEN - 1))){
		cmd_line[i] = cmd_char;
		i++;
	    }
	    break;
	}
    }

    return;
}

static int  telnetd_create(s32 port)
{
    gserver_sock = net_init(port);
    if (gserver_sock <= 0){
	TEL_ERR("%s: gserver_sock=%d\n", __FUNCTION__, gserver_sock);
	return -ENET_INIT;
    }
    
    if (pthread_create(&thread_data.telnetd_handle, NULL, (void*)telnetd_rcv, (void *)(NULL)) != 0)
    {
        return -ETHREAD;
    }
    TEL_PRT("telserv RCV thread create ok\n");

    return EOK;
}

static int telusage(int arg)
{
    int i;
    s8 cmd[ARG_MAX_LEN] = {0};
    s8 usage[USAGE_MAX_LEN] = {0};
    
    tel_print("cmd\t\t\tusage\n");
    for (i=0;i<SUPPORT_CMD_NO;i++){
	if (!cmd_usage(i, cmd, usage)){
	    tel_print(cmd);
	    tel_print("\t\t");
	    tel_print(usage);
	    tel_print("\n");

	    memset(cmd, 0, sizeof(cmd));
	    memset(usage, 0, sizeof(usage));
	}
    }
    return EOK;
}

static int telexit(int arg)
{
    shutdown(gclient_sock, SHUT_RDWR);
    close(gclient_sock);
    gclient_sock = -1;

    return EOK;
}

int tel_reg(s8 *cmd, void *func,  s8 *usage)
{
    return cmd_reg(cmd, func, usage);
}

int tel_init(s32 port)
{
    static int tel_init_flag = 0;
    if(TRUE == tel_init_flag){
	return EOK;
    }
    tel_init_flag = FALSE;

    cmd_init();
    
    tel_reg("exit", (void *)telexit, "exit this client");
    tel_reg("help", (void *)telusage, "list all commands and their usage");
    
    thread_data.telnetd_enable = TRUE;
    pthread_mutex_init(&print_mutex, NULL);
    
    return telnetd_create(port);
}
void tel_exit(void)
{
    thread_data.telnetd_enable = FALSE;
    usleep(200*1000);
    pthread_mutex_destroy(&print_mutex);
    net_exit(gserver_sock);

}
void tel_print(char* fmt, ...)
{
    double vargflt = 0;
    unsigned int  vargint = 0;
    char* vargpch = NULL;
    char vargch = 0;
    char* pfmt = NULL;
    va_list vp;
    pthread_mutex_lock(&print_mutex);
    va_start(vp, fmt);
    pfmt = fmt;

    while(*pfmt)
    {
        if(*pfmt == '%')
        {
            switch(*(++pfmt))
            {
                
                case 'c':
                    vargch = va_arg(vp, int); 
 		    print_char(vargch);
                    break;
                case 'd':
                case 'i':
                    vargint = va_arg(vp, int);
                    print_dec(vargint);
                    break;
                case 'f':
                    vargflt = va_arg(vp, double);
                    print_float(vargflt);
                    break;
                case 's':
                    vargpch = va_arg(vp, char*);
                    print_str(vargpch);
                    break;
                case 'x':
                case 'X':
                    vargint = va_arg(vp, int);
                    print_hex(vargint);
                    break;
                case '%':
                    print_char('%');
                    break;
                default:
                    break;
            }
            pfmt++;
        }
        else
        {
            print_char(*pfmt++);
	    if (*(pfmt-1) == '\n')
		print_char('\r');
        }
    }
    va_end(vp);
    fsync(gclient_sock);
    pthread_mutex_unlock(&print_mutex);
}

#ifdef TEL_DEBUG_DEMO_ON
void zin(void)
{
    tel_print("test.....\n");
}
int main(void)
{
    int ret = EOK;
    int port = 8888;

    ret = tel_init(port);
    if (ret != EOK) {
        TEL_ERR("tel_init with error No.: %d\n", ret);
        return ret;
    }
    
    TEL_PRT("telnetd server started, you may connect it by:  telnet 127.0.0.1 %d\n", port);
        
    tel_reg("zin", (void *)zin, "test...");
    tel_reg("zout", (void *)zin, "test...");
    tel_reg("zin", (void *)zin, "test...");
    tel_reg("zo", (void *)zin, "test...");
    tel_reg("zi", (void *)zin, "test...");
    tel_reg("zt", (void *)zin, "test...");
    tel_reg("znnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn", (void *)zin, "test...");

    /* Just a demo, no exit provided, feel free to kill it by "C-c" */
    while (1){
	usleep(1000*1000);
    }
    
    TEL_PRT("telnet closed\n");
    tel_exit();

    return EOK;
}
#endif
