#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    pid_t wpid;
    int status;
    // 创建子进程
    pid = fork();
    if(pid == -1)
    {
        perror("fork error");
        exit(1);
    }

    // 如果是子进程
    if(pid == 0)
    {
        printf("child pid = %d, going to die 10s\n", getpid());
        sleep(3);
        printf("child died----------------\n");
    }
    else if(pid > 0)    // 父进程
    {
        // 只能回收一个子进程
        wpid = wait(&status);   // 父进程阻塞等待子进程终止，并回收资源
        if(wpid == -1)
        {
            perror("wait error");
            exit(1);
        }
        if(WIFEXITED(status))   // 子进程正常终止
        {
            printf("child exit with %d\n", WEXITSTATUS(status));    // 获取返回值
        }
        if(WIFSIGNALED(status)) // 子进程被信号终止
        {
            printf("child exit by signal %d\n", WTERMSIG(status));
        }
        printf("father exit");
    }
    else
    {
        perror("fork error");
        exit(1);
    }

    exit(0);
}