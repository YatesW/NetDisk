#include "head.h"
int main(int argc ,char **argv)
{
    ARGS_CHECK(argc,3);
    //初始化并建立连接
    int socketfd;
    socketfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(socketfd,-1,"socket");
    printf("socketfd=%d\n",socketfd);
    int ret;
    struct sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(atoi(argv[2]));//字符串转整型，大端转小端，注意不能用htonl
    server.sin_addr.s_addr=inet_addr(argv[1]);//ip地址转32位网络字节序
    ret=connect(socketfd,(struct sockaddr*)&server,sizeof(struct sockaddr));
    ERROR_CHECK(ret,-1,"connect");
    //epoll监控stdin,socketfd
    int epfd=epoll_create(1);
    struct epoll_event event,*evs;
    evs=(struct epoll_event*)calloc(2,sizeof(struct epoll_event));
    event.events=EPOLLIN;
    event.data.fd=STDIN_FILENO;
    epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&event);
    event.data.fd=socketfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,socketfd,&event);
    int readyNum,i,sockdatalen;
    char stdbuf[1024]={0};
    char sockbuf[1024]={0};
    train_t train;
    while(1)
    {
        memset(evs,0,2*sizeof(struct epoll_event));
        readyNum=epoll_wait(epfd,evs,2,-1);
        ERROR_CHECK(readyNum,-1,"epoll_wait");
        for(i=0;i<readyNum;++i)
        {
            if(evs[i].data.fd==STDIN_FILENO)
            {
                memset(stdbuf,0,sizeof(stdbuf));
                train.datalen=read(STDIN_FILENO,stdbuf,sizeof(stdbuf))-1;
                strcpy(train.buf,stdbuf);
                send(socketfd,&train,4+train.datalen,0);//把命令发送过去
                if(strncmp(stdbuf,"puts",4)==0)
                {
                    int j=0,fd;
                    while(stdbuf[j])
                    {
                        stdbuf[j]=stdbuf[j+5];
                        ++j;
                    }
                    stdbuf[strlen(stdbuf)-1]=0;
                    fd=open(stdbuf,O_RDONLY);
                    if(fd==-1)
                    {
                        printf("puts error!please check filename\n");
                    }
                    struct stat statbuf;
                    fstat(fd,&statbuf);
                    memcpy(train.buf,&statbuf.st_size,sizeof(statbuf.st_size));
                    train.datalen=sizeof(statbuf.st_size);
                    send_n(socketfd,(char*)&train,4+train.datalen);//把总文件长度发过去
                    while((train.datalen=read(fd,train.buf,sizeof(train.buf))))
                    {
                        send_n(socketfd,(char*)&train,4+train.datalen);
                    }
                    send_n(socketfd,(char*)&train,4+train.datalen);
                    close(fd);
                    ////循环发送文件内容
                    //train.datalen=1;
                    //while((train.datalen=read(fd,train.buf,sizeof(train.buf))))
                    //{
                    //    send(socketfd,&train,4+train.datalen,0);
                    //}
                    ////发结束标志
                    //train.datalen=0;
                    //send(socketfd,&train,4+train.datalen,0);
                    //close(fd);
                }
            }
            if(evs[i].data.fd!=socketfd)
            {
                system("clear");
            }
            if(evs[i].data.fd==socketfd)
            {
                memset(sockbuf,0,sizeof(sockbuf));
                ret=recv(socketfd,&sockdatalen,sizeof(int),0);
                if(0==ret)
                {
                    printf("server is closed\n");
                    goto end;
                }
                if(strncmp(stdbuf,"gets",4)==0)
                {
                    if(-1==sockdatalen)//get文件错误
                    {
                        printf("Download error!please check file name\n");
                    }else{//读文件
                        recv(socketfd,sockbuf,sockdatalen,0);//接文件名
                        off_t fileTotalSize,fileloadSize=0;
                        recv_n(socketfd,(char*)&sockdatalen,sizeof(int));
                        recv_n(socketfd,(char*)&fileTotalSize,sockdatalen);//接文件总大小
                        int fd=open(sockbuf,O_WRONLY|O_CREAT,0666);
                        time_t start=time(NULL),now;
                        now=start;
                        while(1)
                        {
                            recv_n(socketfd,(char*)&sockdatalen,sizeof(int));
                            if(sockdatalen>0)
                            {
                                recv_n(socketfd,(char*)sockbuf,sockdatalen);
                                write(fd,sockbuf,sockdatalen);
                                fileloadSize+=sockdatalen;
                                now=time(NULL);
                                if(now-start>0)
                                {
                                    printf("%5.2f%s\r",(double)fileloadSize/fileTotalSize*100,"%");
                                    fflush(stdout);
                                    start=now;
                                }
                            }else{
                                printf("100.00%s\n","%");
                                close(fd);
                                printf("recv success\n");
                                break;
                            }
                        }
                    }
                }else{
                    recv(socketfd,sockbuf,sockdatalen,0);
                    printf("%s\n",sockbuf);
                }
            }
        }
    }
end:
    close(socketfd);
    return 0;
}
