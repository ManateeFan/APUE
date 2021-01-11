#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <wait.h>

// 服务器向客户返回文件内容
void client(int readfd, int writefd)
{
    size_t len;
    ssize_t n;
    char buf[128];
    fgets(buf, 128, stdin);
    len = strlen(buf);
    // fgets 会在结尾自动加上'\n'字符
    if (buf[len - 1] == '\n')
        len--;
    write(writefd, buf, len);
    while ((n = read(readfd, buf, 128)) > 0)
    {
        write(STDOUT_FILENO, buf, n);
    }
}

void server(int readfd, int writefd)
{
    int fd;
    ssize_t n;
    char buf[128 + 1];
    if ((n = read(readfd, buf, 128)) == 0)
    {
        fprintf(stderr, "Error path name\n");
        exit(1);
    }
    buf[n] = '\0';
    if ((fd = open(buf, O_RDONLY)) < 0)
    {
        snprintf(buf + n, sizeof(buf) - n, ": can't open, %s\n", strerror(errno));
        n = strlen(buf);
        write(writefd, buf, n);
    }
    else
    {
        while ((n = read(fd, buf, 128)) > 0)
        {
            write(writefd, buf, n);
        }
        close(fd);
    }
}