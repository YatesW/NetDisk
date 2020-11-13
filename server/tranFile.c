#include "head.h"
int tran_File(int newfd,char *fileName)
{
    train_t t;
    //发送文件名
    strcpy(t.buf,fileName);
    t.datalen=strlen(fileName);
    send_n(newfd,(char*)&t,4+t.datalen);
    //发文件内容
    int fd=open(fileName,O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    //发文件大小
    struct stat statbuf;
    fstat(fd,&statbuf);
    memcpy(t.buf,&statbuf.st_size,sizeof(statbuf.st_size));
    t.datalen=sizeof(statbuf.st_size);
    send_n(newfd,(char*)&t,4+t.datalen);
    while((t.datalen=read(fd,t.buf,sizeof(t.buf))))
    {
        send_n(newfd,(char*)&t,4+t.datalen);
    }
    send_n(newfd,(char*)&t,4+t.datalen);//发送结束标志
    return 0;
}

