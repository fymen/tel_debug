#ifndef TEL_DEBUG_H
#define TEL_DEBUG_H

/* error numbers */
#define ECMD_REG_FULL 21        /* command table is full */
#define ECMD_REG_CONFLICT 22    /* this command already
				   defined */
#define ECMD_NOT_EXIST 23       /* no such command */

/**
 *  \brief register a function as a cmd in tel_debug.
 *
 *  the cmd line args will be transformed to function args.
 *
 *  \param cmd, the comand name
 *  \param func, your private function
 *  \param usage, how to use the cmd and what it for
 */
int tel_reg(char *cmd, void *func,  char *usage);
/**
 *  \brief initialize tel_debug
 *  \param port, the server will listen on this port
 */
int tel_init(int port);
/**
 *  \brief exit tel_debug
  */
void tel_exit(void);
/**
 *  \brief print something to tel_debug console
 *  \param just like printf
 */
void tel_print(char* fmt, ...);

#endif
