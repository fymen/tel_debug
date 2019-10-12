#ifndef NET_H
#define NET_H

int net_init(s32 port);
int net_accept(s32 srv_sock);
int net_send(s32 sock, u8 *str, s32 len);
int net_recv1char(s32 sock, u8 *c);
int net_exit(s32 srv_sock);

#endif
