#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int main()
{
    printf("before fork==\n");
    printf("before fork==\n");
    printf("before fork==\n");
    printf("before fork==\n");

    pid_t pid = fork();
    if(pid == -1)
    {
        perror("fork error");
        exit(1);
    }
    else if (pid == 0)
    {   
        printf("This is child, creat  my pid:%d  my parent pid:%d\n", getpid(), getppid());
    }
    else 
    {
        printf("Child is created my pid:%d  my parent pid:%d\n", getpid(), getppid());
    }

    printf("After creating. pid: %d\n", pid);
    
    
    exit(0);
}