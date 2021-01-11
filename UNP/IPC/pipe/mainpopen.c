/* 标准io库提供的函数，创建一个管道，并启动另外一个进程执行命令 */
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

int main()
{
    size_t n;
    char buf[128], command[128];
    FILE *fp;
    // read pathname
    fgets(buf, 128, stdin);
    n = strlen(buf);
    if (buf[n - 1] == '\n')
        n--;
    snprintf(command, sizeof(command), "cat %s", buf);
    // 创建一个执行command命令的进程
    fp = popen(command, "r");
    while (fgets(buf, 128, fp) != NULL)
        fputs(buf, stdout);
    // 等待命令终止
    pclose(fp);
    exit(0);
}