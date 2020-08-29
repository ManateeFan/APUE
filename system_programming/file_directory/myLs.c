#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

// 输出格式
// 文件类型   权限xxx八进制   uid     gid   size    mtime     name

// 出错处理
void sys_err(const char *str);

// 打印一个目录
void printDir(char *dirPath);

// ls -l
void detailDir(char *path);

// 获取文件类型
void file_type(mode_t mode);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        printf("Usage:%s -l path", argv[0]);
        exit(0);
    }
    char dirCur[30];
    // 获取进程当前目录
    if (getcwd(dirCur, 30) == NULL)
        sys_err("getcwd()");

    // ls
    if (argc == 1)
    {
        // 列当前目录
        printDir(dirCur);
    }
    // ls -l dir dir dir ...
    else if (argc == 2 && strcmp(argv[1], "-l") == 0)
    {
        detailDir(dirCur);
    }
    else if (argc > 2)
    {
        for (int i = 2; i < argc; i++)
        {
            printf("%s:\n", argv[i]);
            detailDir(argv[i]);
        }
    }
    else // ls dir
    {
        for (int i = 1; i < argc; i++)
        {
            printf("%s:", argv[i]);
            printDir(argv[i]);
        }
    }

    exit(0);
}

void file_type(mode_t mode)
{
    char *ptr;
    if (S_ISREG(mode)) // 普通文件
        ptr = "Regular";
    else if (S_ISDIR(mode)) // 目录
        ptr = "Directory";
    else if (S_ISCHR(mode)) // 字符处理
        ptr = "character special";
    else if (S_ISBLK(mode)) // 块处理
        ptr = "block special";
    else if (S_ISFIFO(mode)) // fifo
        ptr = "fifo";
    else if (S_ISLNK(mode)) // 链接
        ptr = "symbolic link";
    else if (S_ISSOCK(mode)) // socket
        ptr = "socket";
    else
        ptr = "Unknown type";
    printf("%s\t", ptr);
}

void printDir(char *dirPath)
{
    DIR *pDir = opendir(dirPath);
    if (pDir == NULL)
        sys_err("opendir()");

    // 目录项
    struct dirent *tmpDent;
    int i = 0;
    while (tmpDent = readdir(pDir))
    {
        // 不打印父目录与自己
        if (strcmp(".", tmpDent->d_name) == 0 || strcmp("..", tmpDent->d_name) == 0)
            continue;
        else
            printf("%-s  ", tmpDent->d_name);

        // 一行打印5个
        if (i % 5 == 0)
            printf("\n");
        i++;
    }
    printf("\n");
}

void detailDir(char *path)
{

    printf("%s:\n", path);
    // 获取文件信息结构体
    struct stat statBuf;

    // 打开目录
    DIR *pDir = opendir(path);
    if (pDir == NULL)
        sys_err("opendir()");

    struct dirent *tmpDent;
    // 读目录
    while (tmpDent = readdir(pDir))
    {
        if (strcmp(".", tmpDent->d_name) == 0 || strcmp("..", tmpDent->d_name) == 0)
            continue;

        file_type(statBuf.st_mode);

        // 权限
        printf("%o\t", statBuf.st_mode & 000777);
        // uid  gid
        printf("10%d\t10%d", statBuf.st_uid, statBuf.st_gid);
        // file size
        printf("%12ld\t", statBuf.st_size);
        // 时间
        int ret = stat(tmpDent->d_name, &statBuf);
        time_t t = statBuf.st_mtime;
        struct tm *p = gmtime(&t);
        char s[50];
        strftime(s, 50, "%Y-%m-%d %H:%M:%S", p);
        printf("%s\t", s);

        // 文件名
        printf("%s\n", tmpDent->d_name);
    }
}
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}