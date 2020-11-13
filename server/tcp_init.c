#include "head.h"

int tcp_init(int *socketfd,char *ip,char *port)
{
    //初始化socket
    int fd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(fd,-1,"socket");
    int ret;
    //绑定bind
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(atoi(port));
    addr.sin_addr.s_addr=inet_addr(ip);
    ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"bind");
    //监听listen
    ret=listen(fd,BACKLOG);
    ERROR_CHECK(ret,-1,"listen");
    //
    *socketfd=fd;
    return 0;
}

