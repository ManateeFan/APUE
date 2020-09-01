#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    char buf[20];

    fd = open("data.txt", O_RDONLY);
    if (fd == -1)
    {
        perror("open error");
        return 1;
    }

    printf("file descriptor: %d\n", fd);

    if (read(fd, buf, sizeof(buf)) == -1)
    {
        perror("write error");
        return 1;
    }
    printf("file data: %s", buf);
    close(fd);
    return 0;
}