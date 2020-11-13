#include<func.h>

typedef struct{
    int datalen;//文件长度
    char buf[1024];//装文件内容
}train_t;
int send_n(int sfd,char *ptran,int len);
int recv_n(int sfd,char *ptran,int len);
