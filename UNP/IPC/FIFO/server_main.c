#include "fifo.h"

int main()
{
    int readfd, writefd;
    mkfifo(FIFO1, FILE_MODE);
    mkfifo(FIFO2, FILE_MODE);
    readfd = open(FIFO1, O_RDONLY, 0);
    writefd = open(FIFO2, O_WRONLY, 0);
    server(readfd, writefd);
    exit(0);
}