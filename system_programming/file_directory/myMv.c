#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if(argc < 3 || strcmp(argv[1], "--help") ==0)
    {
        printf("Usage: %s oldName newName", argv[0]);
    }
    // 建立硬连接，实质是建立一个新的dentry
    int res = link(argv[1], argv[2]);
    if(res == -1)
    {
        perror("Mv error");
        exit(1);
    }
    // 删除硬连接，实质是删除一个dentry
    int del = unlink(argv[1]);
    if(del == -1)
    {
        perror("del error");
        exit(1);
    }
    exit(0);
}
