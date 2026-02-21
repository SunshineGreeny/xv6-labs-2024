#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path,const char *filename){
    char buf[512],*p;
    int fd; //文件描述符，用于打开目录
    struct dirent de; //目录项结构体，存储目录中每个条目（文件/子目录）的信息
    struct stat st; //文件状态结构体，存储文件的元数据（类型、大小等）
    
    //尝试以只读模式（标志位 0）打开路径
    if((fd = open(path,0))<0){
        fprintf(2,"find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd,&st)<0){
        fprintf(2,"find: cannot open %s\n", path);
        close(fd);
        return;
    }

    //参数错误，find的第一个参数必须是目录
    if(st.type!=T_DIR){ //T_DIR 是目录类型常量
        //如果不是目录，打印使用说明
        fprintf(2,"usage: find <DIRECTORY> <filename>\n");
        return;
    }

    //检查拼接后的路径是否会超出缓冲区大小
    if(strlen(path)+1+DIRSIZ+1>sizeof buf){
        fprintf(2, "find: path too long\n");
        return;
    }
    strcpy(buf,path); //将传入的路径复制到缓冲区 buf
    p=buf+strlen(buf); //指针 p 指向 buf 字符串末尾（准备追加内容）
    *p++ = '/'; //p指向最后一个'/'之后

    //循环读取目录内容
    while(read(fd,&de,sizeof(de))==sizeof(de)){
        //跳过无效目录项
        if(de.inum==0) //inum 是 inode 编号，0 表示该目录项未被使用（空条目）
            continue;
        memmove(p,de.name,DIRSIZ); //添加路径名称
        p[DIRSIZ]=0; //字符串结束标志

        //获取文件状态
        if(stat(buf, &st)<0){
            fprintf(2, "find: cannot stat %s\n",buf);
            continue;
        }

        //递归搜索子目录
        if(st.type == T_DIR && strcmp(p, ".")!=0 && strcmp(p, "..")!=0){ //不要在"."和".."目录中递归
            find(buf, filename);
        } 
        //匹配目标文件
        else if(strcmp(filename, p)==0){
            printf("%s\n",buf);
        }
    }
    close(fd);
}

int main(int argc,char *argv[]){
    //检查参数个数是否为 3（程序名 + 目录 + 文件名）
    if(argc!=3){
        //如果参数不对，打印使用说明
        fprintf(2, "usage: find <directory> <filename>\n");
        exit(1);
    }
    find(argv[1],argv[2]);
    exit(0);
}