#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;

#define NEWLINE_CHAR  '\n'
#define BACKSPACE_CHAR  8
#define BLANK_CHAR  ' '
#define RETURN_CHAR  13
#define TAB_CHAR  9
#define DEL_CHAR  127
#define CTRL_S  0x13
#define CTRL_R  0x12
#define CTRL_C  0x03
#define CTRL_Z  0x1A
#define UP_ARROW  27
#define DOWN_ARROW  28
#define LEFT_ARROW  29
#define RIGHT_ARROW  30

#define TELCMD_WILL    (u8)251
#define TELCMD_WONT    (u8)252
#define TELCMD_DO      (u8)253
#define TELCMD_DONT    (u8)254
#define TELCMD_IAC     (u8)255
#define TELCMD_EC     (u8)0xF7
#define TELCMD_EL     (u8)0xF8

#define TELOPT_ECHO     (u8)1
#define TELOPT_SGA      (u8)3
#define TELOPT_LFLOW    (u8)33
#define TELOPT_NAWS     (u8)34

#define TRUE 1
#define FALSE 0

#define EOK 0
#define EARG 1
#define EBIND 2
#define ELISTEN 3
#define ESOCKET 4
#define ENET_INIT 5

#define ETHREAD 11


#define debug_flag 0

#define TEL_PRT(fmt, args...) {if (debug_flag) {fprintf(stdout, "tel: "); fprintf(stdout, fmt, ## args);}}
#define TEL_ERR(fmt, args...) {if (debug_flag) {fprintf(stderr, "tel_err: "); fprintf(stderr, fmt, ## args);}}

#endif
