#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int main()
{
    int i = 0;

    for (; i < 5; i++)
    {
        if (fork() == 0) // 防止子进程继续创建它的子进程
            break;       // 保证只有父进程一个在创建进程
    }

    if (i == 5)
        printf("i am father\n");
    else
    {
        sleep(i + 1);
        printf("i am %dth created\n", i + 1);
    }
    exit(0);
}