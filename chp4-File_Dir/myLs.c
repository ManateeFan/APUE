#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // stat()
#include <unistd.h>

int isDir(char* fileName); // 判断是否是目录
void listDir(char* fileName); // 打印目录主函数

int main(int argc, char* argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
    {
        printf("Usage : %s dir", argv[0]);
        exit(0);
    }

    listDir(argv[1]);

    exit(0);
}

int isDir(char* fileName)
{
    struct stat tmp;
    // 获取文件状态信息，放入tmp结构
    if (stat(fileName, &tmp) == -1)
    {
        perror("stat");
        exit(1);
    }
    // 是否为目录
    if (S_ISDIR(tmp.st_mode))
        return 1;
    return 0;
}

void listDir(char* fileName)
{
    DIR* dirp = NULL;
    // 打开目录
    if ((dirp = opendir(fileName)) == NULL)
    {
        perror("opendir()");
        exit(1);
    }

    // 获取当前工作目录
    char curDir[256];
    if (getcwd(curDir, 256) == NULL)
    {
        perror("getcwd()");
        exit(1);
    }

    struct dirent* dtp = NULL;
    while ((dtp = readdir(dirp)) != NULL)
    {
        char tmpName[256];
        // 获取该目录项中的文件名
        char* name = dtp->d_name;
        if (name[0] == '.')
            continue;
        // tmpName为该文件的绝对目录
        fprintf(tmpName, "%s/%s", curDir, name);
        // 如果不是目录
        if (isDir(tmpName) == 0)
        {
            printf("%s\n", tmpName);
        }
        else
        {
            listDir(tmpName);
        }
    }
}