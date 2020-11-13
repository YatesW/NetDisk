#include "head.h"
//创建num个子进程并初始化
int makechild(proData_t *p,int num)
{
    int i=0;
    int fds[2];
    pid_t pid;
    for(i=0;i<num;++i)
    {
        socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        pid=fork();
        if(0==pid)
        {
            close(fds[1]);
            childHandle(fds[0]);//实际上每次传进来的都是3
        }
        close(fds[0]);
        p[i].pid=pid;
        p[i].fd=fds[1];
        p[i].busy=0;
    }
    return 0;
}
void childHandle(int fd)
{
    char flag=1;
    char buf[1024];
    int newfd,ret,datalen;
    while(1)
    {
        recv_fd(fd,&newfd);
        if(-1==newfd)
        {
            exit(0);//子进程要退出了
        }
        while(1)
        {
            memset(buf,0,sizeof(buf));
            ret=recv(newfd,&datalen,sizeof(int),0);
            if(ret==0)
            {
                break;
            }
            ret=recv(newfd,buf,datalen,0);
            if(ret==0)
            {
                break;
            }
            if(strncmp(buf,"ls",2)==0)
            {
                do_ls(newfd);
            }else if(strncmp(buf,"pwd",3)==0)
            {
                do_pwd(newfd);
            }else if(strncmp(buf,"cd",2)==0)
            {
                do_cd(newfd,buf);
            }else if(strncmp(buf,"remove",6)==0)
            {
                do_remove(newfd,buf);
            }else if(strncmp(buf,"puts",4)==0)
            {
                do_puts(newfd,buf);
            }else if(strncmp(buf,"gets",4)==0)
            {
                do_gets(newfd,buf);
            }
        }
        write(fd,&flag,sizeof(char));//通知父进程完成任务
    }
}

