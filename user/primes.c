#include "kernel/types.h"
#include "user/user.h"

#define RD 0
#define WR 1

const uint INT_LEN = sizeof(int);

//从左管道读取第一个数据
int lpipe_first_data(int lpipe[2],int *dst){
    if(read(lpipe[RD],dst,sizeof(int))==sizeof(int)){
        printf("prime %d\n",*dst);
        return 0;
    }
    return -1;
}

//从左管道读取数据，过滤后写入右管道
void transmit_data(int lpipe[2], int rpipe[2], int first){
    int data;
    //循环从左管道读取所有剩余数据
    while(read(lpipe[RD],&data,sizeof(int))==sizeof(int)){
        if(data%first){ //将无法整除的数据传递入右管道
            write(rpipe[WR],&data,sizeof(int)); //将候选数据写入右管道，传递给下一个进程
        }
    }
    close(lpipe[RD]);
    close(rpipe[WR]);
}

void primes(int lpipe[2]){
    int first;
    if(lpipe_first_data(lpipe,&first)==0){
        int p[2];
        pipe(p);

        if(fork()==0){
            close(p[WR]); //子进程关闭右管道写端
            close(lpipe[RD]); //子进程不需要读老管道了（父进程在读），关闭防泄漏
            primes(p);
        }else{
            close(p[RD]); //父进程关闭右管道读端
            //父进程去执行传输过滤任务，这样子进程可以同时在后台读取，实现并发！
            transmit_data(lpipe,p,first);
            wait(0);
        }
    }
    else{ //如果管道空了，关闭读段
        close(lpipe[RD]);
    }
    exit(0);
}

int main(int argc,char const *argv[]){
    int p[2];
    pipe(p);

    if(fork()==0){ 
        close(p[WR]); //子进程关闭写端
        primes(p);
    }else{ 
        close(p[RD]); //父进程关闭读端

        //先把子进程 fork 出来，再开始大量写入数据。
        //这样即使数据超过管道大小，子进程也会一边消耗一边腾出空间，不会死锁。
        for(int i=2;i<=280;i++){
            write(p[WR],&i,INT_LEN);
        }
        close(p[WR]); //写入完毕，关闭写端，这会向流水线发送 EOF 信号
        wait(0); //等待子进程链全部结束
    }

    exit(0);
}