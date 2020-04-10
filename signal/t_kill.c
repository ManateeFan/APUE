#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void printSet(const sigset_t *set)
{
    int i;
    for (i = 1; i < 32; i++)
    {
        if (sigismember(set, i))
            putchar('1');
        else
            putchar('0');
    }
    printf("\n");
}
int main()
{
    // 信号集
    sigset_t set, oldset;
    int ret = 0;

    // 初始化信号集
    sigemptyset(&set);
    // 添加信号
    sigaddset(&set, SIGINT);
    // 阻塞
    ret = sigprocmask(SIG_BLOCK, &set, &oldset);
    if (ret == -1)
    {
        perror("sigprocmask()");
        exit(EXIT_FAILURE);
    }

    sigset_t pedset;
    // 查看未决信号集
    while (1)
    {
        ret = sigpending(&pedset);
        if (ret == -1)
        {
            perror("sigpending()");
            exit(EXIT_FAILURE);
        }
        printSet(&pedset);
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}
