#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <wait.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"

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

int main()
{
    int readfd, writefd;
    pid_t child;
    // create FIFO
    mkfifo(FIFO1, FILE_MODE);
    mkfifo(FIFO2, FILE_MODE);
    if ((child = fork()) == 0)
    {
        // 如果当前没进程以写方式打开某个FIFO，那么以读打开该FIFO的进程将阻塞
        readfd = open(FIFO1, O_RDONLY, 0);
        writefd = open(FIFO2, O_WRONLY, 0);
        server(readfd, writefd);
        exit(0);
    }

    /* error */
    // block here
    // readfd = open(FIFO2, O_RDONLY, 0);
    //
    // writefd = open(FIFO1, O_WRONLY, 0);

    // open with write
    writefd = open(FIFO1, O_WRONLY, 0);
    // open with read
    readfd = open(FIFO2, O_RDONLY, 0);
    client(readfd, writefd);
    waitpid(child, NULL, 0);
    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);

    exit(0);
}