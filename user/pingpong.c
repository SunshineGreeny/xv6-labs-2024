#include "kernel/types.h"
#include "user/user.h"

#define RD 0 //pipe的read端
#define WR 1 //pipe的write端

int main(int argc, char const *argv[]){
    char buf='P'; //用于传送的字节

    int fd_c2p[2]; //子进程->父进程
    int fd_p2c[2]; //父进程->子进程

    //创建管道
    pipe(fd_c2p);
    pipe(fd_p2c);

    //创建子进程
    int pid=fork();
    int exit_status=0;

    if(pid<0){
        fprintf(2,"fork() error!\n");
        close(fd_c2p[RD]);
        close(fd_c2p[WR]);
        close(fd_p2c[RD]);
        close(fd_p2c[WR]);
        exit(1);
    }
    else if(pid==0){ //子进程
        close(fd_p2c[WR]); //关闭父->子管道的写端（子进程只需要从这个管道读）
        close(fd_c2p[RD]); //关闭子->父管道的读端（子进程只需要向这个管道写）

        //读取父进程的 ping
        if(read(fd_p2c[RD],&buf,sizeof(char))!=sizeof(char)){ //读取失败（返回值不是 1）
            fprintf(2,"child read() error!\n");
            exit_status=1; //出错
        }else{
            fprintf(1,"%d:received ping\n",getpid());
        }

        //向父进程发送 pong
        if(write(fd_c2p[WR],&buf,sizeof(char))!=sizeof(char)){
            fprintf(2,"child write() error!\n");
            exit_status=1; //出错            
        }

        //关闭剩余端口并退出
        close(fd_c2p[RD]);
        close(fd_p2c[WR]);

        exit(exit_status);
    }
    else{ //父进程
        close(fd_p2c[RD]); //关闭父->子管道的读端（父进程只需要向这个管道写）
        close(fd_c2p[WR]); //关闭子->父管道的写端（父进程只需要向这个管道读）

        //向子进程发送 ping
        if(write(fd_p2c[WR],&buf,sizeof(char))!=sizeof(char)){
            fprintf(2,"parent write() error!\n");
            exit_status=1; //出错            
        }

        //读取子进程的 pong
        if(read(fd_c2p[RD],&buf,sizeof(char))!=sizeof(char)){ //读取失败（返回值不是 1）
            fprintf(2,"parent read() error!\n");
            exit_status=1; //出错
        }else{
            fprintf(1,"%d:received pong\n",getpid());
        }

        //关闭剩余端口并退出
        close(fd_c2p[RD]);
        close(fd_p2c[WR]);

        exit(exit_status);
    }
}