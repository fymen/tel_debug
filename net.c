#include "common.h"
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "net.h"
#include <errno.h>

int net_init(s32 port)
{
    s32 sock;
    s32 opt = 1;
    struct sockaddr_in in_addr;
    int ret;
    
    if ((port < 3000) || (port > 60000)){
	TEL_ERR("port range: %d-%d\n", 3000, 60000);
	return -EARG;
    }
    memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET; 
    in_addr.sin_port = htons(port);
    in_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
	TEL_ERR("create socket error\n");
	return -ESOCKET;
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sock, (struct sockaddr *)&in_addr, sizeof(in_addr)) < 0){
	TEL_ERR("bind socket error\n");
	close(sock);
	return -EBIND;
    }
    
    ret = listen(sock, 15);
    if (ret < 0){
	TEL_ERR("listen on port: %d error,ret=%d\n", port, errno);
	close(sock);
	return -ELISTEN;
    }
        
    return sock;
}

int net_accept(s32 srv_sock)
{
    s32 sock, ret;
    struct sockaddr_in addrClient;
    u32 addr_len = sizeof(addrClient);
    
    fd_set rfds;
    struct timeval tv;

    sock = -1;
    
    if (srv_sock <= 0){
	TEL_ERR("srv_sock %d\n", srv_sock);
	return -EARG;
    }

    FD_ZERO(&rfds);
    tv.tv_sec = 50;
    tv.tv_usec = 0;

    FD_SET(srv_sock, &rfds);
    ret = select(srv_sock + 1, &rfds, NULL, NULL, &tv);
    if (ret < 0)
	return ret;
    if(FD_ISSET(srv_sock, &rfds)){
	sock = accept(srv_sock, (struct sockaddr *)&addrClient, &addr_len);	
    }
    
    return sock;
}

int net_send(s32 sock, u8 *str, s32 len)
{
    if (sock <= 0){
	return -EARG;
    }
    
    return send(sock, str, len, 0);
}

int net_recv1char(s32 sock, u8 *c)
{
    if (sock <= 0)
	return -EARG;

    return recv(sock, c, 1, 0);
}

int net_exit(s32 srv_sock)
{
    if (srv_sock <= 0)
	return -EARG;
    return close(srv_sock);
}


