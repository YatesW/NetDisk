#include<func.h>
#define BACKLOG 10//一次tcp连接的最大数量
typedef struct{
    pid_t pid;//子进程id
    int fd;//子进程管道对端
    short busy;//0表示不忙，1表示忙
}proData_t;

typedef struct{
    int datalen;//文件长度
    char buf[1024];//装文件内容
}train_t;

void childHandle(int fd);
int makechild(proData_t*,int);
int tcp_init(int *socketfd,char *ip,char *port);
void send_fd(int socketpipew,int fd);
void recv_fd(int socketpiper,int *fd);
int tran_File(int newfd,char *fileName);
int send_n(int sfd,char *ptran,int len);
int recv_n(int sfd,char *ptran,int len);
void do_ls(int newfd);
void do_pwd(int newfd);
void do_cd(int newfd,char *pathName);
void do_remove(int newfd,char *fileName);
int do_puts(int newfd,char *fileName);
int do_gets(int newfd,char *fileName);

