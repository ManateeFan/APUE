#include <fcntl.h>  // open(), creat()
#include <stdlib.h>
#include <stdio.h>  // perror()
#include <errno.h>  // errno
#include <unistd.h> // read(), write(), close(), lseek()
#include <string.h> // strerror()

#define BUFSIZE 1024
int main(int argc, char* argv[])
{
    int sfd, dfd;
    char buf[BUFSIZE];

    if ((sfd = open(argv[1], O_RDONLY)) < 0)
    {
        fprintf(stderr, "open()1 %s", strerror(errno));
        exit(1);
    }
    if ((dfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, FILESEC_MODE)) < 0)
    {
        fprintf(stderr, "open()2 %s", strerror(errno));
        exit(1);
    }
    int n;
    while ((n = read(sfd, buf, BUFSIZE)) > 0)
    {
        if (write(dfd, buf, n) != n)
        {
            perror("write()");
        }
    }

    close(sfd);
    close(dfd);
    exit(0);
}
