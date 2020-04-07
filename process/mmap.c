#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void sys_err(const char* str)
{
    perror(str);
    exit(1);
}

int main()
{
    // 接收映射区的首地址
    char* p = NULL;
    // file discripter
    int fd;

    fd = open("testmmap", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        sys_err("open()");
    }

    /* lseek 设置文件大小 21B
    lseek(fd, 20, SEEK_END);
    write(fd, "\0", 1);
    */

    // ftruncate 设置文件大小 20B
    ftruncate(fd, 20);
    int len = lseek(fd, 0, SEEK_END);

    // 创建映射区
    p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        sys_err("mmap()");

    // 使用 指针 p 对文件进行读写操作
    strcpy(p, "Hello, mmap");
    printf("%s", p);

    // munmap 释放映射区
    int ret = munmap(p, len);
    if (ret == -1)
        sys_err("munmap()");

    exit(0);
}
