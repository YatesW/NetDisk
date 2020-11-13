#include "head.h"
char* file_type(mode_t t)
{
    if(S_ISREG(t))//是否是普通文件
    {
        return "-";
    }else if(S_ISLNK(t))//是否是一个链接
    {
        return "l";
    }else if(S_ISDIR(t))//是否是一个目录
    {
        return "d";
    }else if(S_ISCHR(t))//是否是字符设备
    {
        return "c";
    }else if(S_ISBLK(t))//是否是块设备
    {
        return "b";
    }else if(S_ISSOCK(t))//是否是套接字
    {
        return "s";
    }else
    {
        return "o";
    }
}
#define GET_FILENAME(len,fileName){int i=0;while(fileName[i]){fileName[i]=fileName[i+len];++i;}}

void do_ls(int newfd)
{
    DIR* pdir=opendir("./");
    if(pdir==NULL)
    {
        printf("opendir error!\n");
    }else
    {
        struct dirent *pbuf;
        struct stat fileinfo;
        train_t train;
        char str[512];
        char* type;
        while((pbuf=readdir(pdir))!=NULL)
        {
            if(strcmp(pbuf->d_name,".")==0||strcmp(pbuf->d_name,"..")==0)
            {
                continue;
            }
            bzero(&fileinfo,sizeof(struct stat));
            stat(pbuf->d_name,&fileinfo);
            type=file_type(fileinfo.st_mode);
            sprintf(str,"%-2s%-20s %10ldB",type,pbuf->d_name,fileinfo.st_size);
            train.datalen=strlen(str);
            strcpy(train.buf,str);
            send(newfd,&train,train.datalen+4,0);
        }
    }
}
void do_pwd(int newfd)
{
    char buf[128]={0};
    getcwd(buf,sizeof(buf));
    train_t train;
    train.datalen=strlen(buf);
    strcpy(train.buf,buf);
    send(newfd,&train,4+train.datalen,0);
}
void do_cd(int newfd,char *pathName)
{
    GET_FILENAME(3,pathName)
    int ret=chdir(pathName);
    if(ret==-1)
    {
        char error[100]="error!please check dir name";
        train_t train;
        train.datalen=sizeof(error);
        strcpy(train.buf,error);
        send(newfd,&train,train.datalen+4,0);
    }
}
void do_remove(int newfd,char *fileName)
{
    int ret;
    train_t train;
    GET_FILENAME(7,fileName)
        if(strncmp(fileName,"*",1)==0)
        {
            DIR* pdir=opendir("./");
            struct dirent *pbuf;
            char last=fileName[strlen(fileName)-1];
            char name[100]={0};
            int j;
            int flag=0;
            while((pbuf=readdir(pdir))!=NULL)
            {
                strcpy(name,pbuf->d_name);
                j=strlen(name)-2;
                if(name[j]=='.'&&name[j+1]==last)
                {
                    unlink(pbuf->d_name);
                    flag=1;
                }
            }
            if(flag==0)
            {
                char error[100]="Delete error!";
                train.datalen=sizeof(error);
                strcpy(train.buf,error);
                send(newfd,&train,train.datalen+4,0);
            }
        }else{
            ret=unlink(fileName);
            if(ret==-1)
            {
                char error[100]="Delete error!please check file name";
                train.datalen=sizeof(error);
                strcpy(train.buf,error);
                send(newfd,&train,train.datalen+4,0);
            }
        }
}
int do_puts(int newfd,char *fileName)
{
    GET_FILENAME(5,fileName);
    int fd,datalen;
    train_t train;
    char buf[1024]={0};
    off_t fileTotalSize,fileloadSize=0;
    recv_n(newfd,(char*)&datalen,sizeof(int));
    recv_n(newfd,(char*)&fileTotalSize,datalen);//接文件总大小
    fd=open(fileName,O_WRONLY|O_CREAT,0666);
    if(-1==fd)
    {
        char error[100]="Upload error!please check file name";
        train.datalen=sizeof(error);
        strcpy(train.buf,error);
        send(newfd,&train,train.datalen+4,0);
        return -1;
    }
    time_t start=time(NULL),now;
    now=start;
    while(1)
    {
        recv_n(newfd,(char*)&datalen,sizeof(int));
        if(datalen>0)
        {
            recv_n(newfd,buf,datalen);
            write(fd,buf,datalen);
            fileloadSize+=datalen;
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
    return 0;
}
int do_gets(int newfd,char *fileName)
{
    GET_FILENAME(5,fileName);
    train_t train;
    int fd=open(fileName,O_RDONLY);
    if(-1==fd)
    {
        train.datalen=-1;
        send(newfd,&train,4,0);
        return -1;
    }
    tran_File(newfd,fileName);
    //train.datalen=strlen(fileName);
    //strcpy(train.buf,fileName);
    //send(newfd,&train,4+train.datalen,0);//发文件名
    //while((train.datalen=read(fd,train.buf,sizeof(train.buf))))
    //{
    //    send(newfd,&train,4+train.datalen,0);
    //}
    ////发文件结束标志
    //send(newfd,&train,4,0);
    return 0;
}
