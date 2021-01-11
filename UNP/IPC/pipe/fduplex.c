#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int fd[2], n;
    char c;
    pid_t child;
    pipe(fd);
    if ((child = fork()) == 0)
    {
        sleep(3);
        if ((n = read(fd[0], &c, 1)) != 1)
        {
            fprintf(stderr, "child : read error\n");
            exit(1);
        }
        printf("child read %c\n", c);
        write(fd[0], "c", 1);
        exit(0);
    }
    write(fd[1], "p", 1);
    if ((n = read(fd[1], &c, 1)) != 1)
    {
        fprintf(stderr, "parent : read error\n");
        exit(1);
    }
    printf("parent read %c\n", c);
    exit(0);
}