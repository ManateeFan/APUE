#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

char buf1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
char buf2[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};

int main()
{
    int fd;

    if((fd = creat("file.hole", FILESEC_MODE)) < 0)
    {
        perror("creat()");
        exit(1);
    }

    if((write(fd, buf1, 10)) != 10)
    {
        perror("buf1 write()");
        exit(1);
    }
    // offset now = 10
    if((lseek(fd, 16384, SEEK_SET)) == -1)
    {
        perror("lseek()");
        exit(1);
    }
    // offset now = 16384
    if((write(fd, buf2, 10)) != 10)
    {
        perror("buf2 write()");
        exit(1);
    }
    // offset now = 16394
    exit(0);
}