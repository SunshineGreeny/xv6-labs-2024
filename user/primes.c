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
    close(lpipe[WR]);
    int first;
    if(lpipe_first_data(lpipe,&first)==0){
        int p[2];
        pipe(p);
        transmit_data(lpipe,p,first);

        if(fork()==0){
            primes(p);
        }else{
            close(p[RD]);
            wait(0);
        }
    }
    exit(0);
}

int main(int argc,char const *argv[]){
    int p[2];
    pipe(p);

    for(int i=2;i<=35;i++){
        write(p[WR],&i,INT_LEN);
    }

    if(fork()==0){ //子进程调用 primes(p) 开始素数筛选
        primes(p);
    }else{ //父进程关闭管道两端（父进程只负责等待）
        close(p[WR]);
        close(p[RD]);
        wait(0);
    }

    exit(0);
}