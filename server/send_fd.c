#include "head.h"
void send_fd(int socketpipew,int fd)
{
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    char buf2[10]="da";
    struct iovec iov[2];
    iov[0].iov_base=&fd;
    iov[0].iov_len=4;
    iov[1].iov_base=buf2;
    iov[1].iov_len=10;
    msg.msg_iov=iov;
    msg.msg_iovlen=2;
    struct cmsghdr *cmsg;
    int len=CMSG_LEN(sizeof(int));
    cmsg=(struct cmsghdr *)calloc(1,len);
    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;
    *(int*)CMSG_DATA(cmsg)=fd;
    if(-1==fd)
    {
        *(int*)CMSG_DATA(cmsg)=0;
    }
    msg.msg_control=cmsg;//嵌套结构体，易忘
    msg.msg_controllen=len;//易忘
    int ret;
    ret=sendmsg(socketpipew,&msg,0);
    if(-1==ret)
    {
        perror("sendmsg");
        return;
    }
}
void recv_fd(int socketpiper,int *fd)
{
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    int exitflag;
    char buf2[10]={0};
    struct iovec iov[2];
    iov[0].iov_base=&exitflag;
    iov[0].iov_len=4;
    iov[1].iov_base=buf2;
    iov[1].iov_len=10;
    msg.msg_iov=iov;
    msg.msg_iovlen=2;
    struct cmsghdr *cmsg;
    int len=CMSG_LEN(sizeof(int));
    cmsg=(struct cmsghdr *)calloc(1,len);
    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;
    msg.msg_control=cmsg;//嵌套结构体，易忘
    msg.msg_controllen=len;//易忘
    int ret;
    ret=recvmsg(socketpiper,&msg,0);
    if(-1==ret)
    {
        perror("sendmsg");
        return;
    }
    *fd=*(int*)CMSG_DATA(cmsg);//把拿到的fd值写出去
    if(-1==exitflag)//此时传过来的是退出标志
    {
        *fd=-1;
    }
}

