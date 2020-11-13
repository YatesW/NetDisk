#include "head.h"
int exitfds[2];
void sigfunc(int signum)
{
    write(exitfds[1],&signum,1);
}
int main(int argc,char **argv)
{
    if(argc!=4)
    {
        printf("serverIP,port,childprocess num\n");
        return -1;
    }
    pipe(exitfds);
    signal(SIGUSR1,sigfunc);
    int childProNum=atoi(argv[3]);
    proData_t *pManage=(proData_t *)calloc(childProNum,sizeof(proData_t));
    makechild(pManage,childProNum);
    int socketfd;
    int ret;
    ret=tcp_init(&socketfd,argv[1],argv[2]);
    if(-1==ret)
    {
        printf("tcp_init error!\n");
        return -1;
    }
    printf("socketfd=%d\n",socketfd);
    //epoll监控socketfd,每一子进程管道对端和退出信号
    struct epoll_event event,*evs;
    evs=(struct epoll_event*)calloc(childProNum+2,sizeof(struct epoll_event));
    int epfd=epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create");
    event.events=EPOLLIN;
    event.data.fd=socketfd;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,socketfd,&event);
    ERROR_CHECK(ret,-1,"epoll_create1");
    event.data.fd=exitfds[0];//监控是否可读，若可读代表要退出了
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,exitfds[0],&event);
    ERROR_CHECK(ret,-1,"epoll_create2");
    int i,j;
    for(i=0;i<childProNum;++i)
    {
        event.data.fd=pManage[i].fd;
        epoll_ctl(epfd,EPOLL_CTL_ADD,pManage[i].fd,&event);
    }
    int fdReadyNum;
    int newfd;
    char flag;
    while(1)
    {
        memset(evs,0,(childProNum+2)*sizeof(struct epoll_event));
        fdReadyNum=epoll_wait(epfd,evs,childProNum+2,-1);
        for(i=0;i<fdReadyNum;++i)
        {
            if(evs[i].data.fd==socketfd)
            {
                newfd=accept(socketfd,NULL,NULL);
                for(j=0;j<childProNum;++j)
                {
                    if(0==pManage[j].busy)
                    {
                        send_fd(pManage[j].fd,newfd);
                        close(newfd);
                        pManage[j].busy=1;
                        printf("%d is busy\n",pManage[j].pid);
                        break;
                    }
                }
            }
            for(j=0;j<childProNum;++j)
            {
                if(pManage[j].fd==evs[i].data.fd)
                {
                    pManage[j].busy=0;
                    ret=read(pManage[j].fd,&flag,sizeof(char));
                    printf("%d is not busy\n",pManage[j].pid);
                    break;
                }
            }
            //当进入后代表要退出了
            if(exitfds[0]==evs[i].data.fd)
            {
                event.events=EPOLLIN;
                event.data.fd=socketfd;
                epoll_ctl(epfd,EPOLL_CTL_DEL,socketfd,&event);
                close(socketfd);
                for(j=0;j<childProNum;++j)
                {
                    send_fd(pManage[j].fd,-1);
                }
                for(j=0;j<childProNum;++j)
                {
                    wait(NULL);
                    printf("child exit success\n");
                }
                exit(0);
            }
        }
    }
    return 0;
}

