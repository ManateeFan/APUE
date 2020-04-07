#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // exec pipe

int main()
{
    pid_t pid;
    int fds[2];
    int ret;

    ret = pipe(fds); // create pipe
    if (ret == -1)
    {
        perror("pipe()");
        exit(1);
    }

    int i;
    for (i = 0; i < 2; i++)
    {
        pid = fork();
        if (pid == 0)
            break;
    }
    if (i == 2) // father
    {
        close(fds[0]);
        close(fds[1]);
        wait(NULL);
        wait(NULL);
    }
    else if (i == 1) // bro one
    {
        close(fds[1]);
        dup2(fds[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
    }
    else // bro two
    {
        close(fds[0]); // close read
        dup2(fds[1], STDOUT_FILENO);
        execlp("ls", "ls", "-l", NULL);
    }
    exit(0);
}