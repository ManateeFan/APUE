#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// 根据命令行参数打印文件类型

int main(int argc, char* argv[])
{
    char* ptr = NULL;
    int i;

    // stat 结构获取st_mode参数
    struct stat fileStat;

    for (i = 1; i < argc; i++)
    {
        printf("%s:", argv[i]);

        if (lstat(argv[i], &fileStat) < 0)
        {
            perror("lstat()");
            continue;
        }

        // 根据st_mode 判断类型
        if (S_ISREG(fileStat.st_mode))
            ptr = "Regular";
        else if (S_ISDIR(fileStat.st_mode))
            ptr = "Directory";
        else if (S_ISCHR(fileStat.st_mode))
            ptr = "character special";
        else if (S_ISBLK(fileStat.st_mode))
            ptr = "block special";
        else if (S_ISFIFO(fileStat.st_mode))
            ptr = "fifo";
        else if (S_ISLNK(fileStat.st_mode))
            ptr = "symbolic link";
        else if (S_ISSOCK(fileStat.st_mode))
            ptr = "socket";
        else
            ptr = "** Unknown mode **";
        printf("%s\n", ptr);
    }
    exit(0);
}