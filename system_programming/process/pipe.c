#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // exec pipe

int main()
{
    pid_t pid;
    int ret;
    int fds[2];

    ret = pipe(fds); // create pipe
    if (ret == -1)
    {
        perror("pipe()");
        exit(1);
    }

    pid = fork(); // create child
    if (pid == -1)
    {
        perror("fork()");
        exit(1);
    }
    else if (pid > 0) // father
    {
        close(fds[1]); // close write
        dup2(fds[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
    }
    else if (pid == 0)
    {
        close(fds[0]); // close read
        int tmpFd = dup2(fds[1], STDOUT_FILENO);
        if (tmpFd == -1)
        {
            perror("dup2()");
            exit(1);
        }
        execlp("ls", "ls", "-l", NULL);
    }
    else
    {
        perror("fork()");
        exit(1);
    }
    exit(0);
}
