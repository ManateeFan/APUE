#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    const char buf[] = "Let's go!\n";

    fd = open("data.txt", O_CREAT | O_WRONLY | O_TRUNC);
    if (fd == -1)
    {
        perror("open error");
        return 1;
    }
    printf("file descriptor: %d\n", fd);

    if (write(fd, buf, sizeof(buf)) == -1)
    {
        perror("write error");
        return 1;
    }
    close(fd);
    return 0;
}