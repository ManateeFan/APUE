#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

// -e 命令选项来 制定IPC_EXCL标志
int main(int argc, char *argv[])
{
    int msqid;
    int oflag = 0664 | IPC_CREAT;

    int c = 0;
    while ((c = getopt(argc, argv, "e")) != -1)
    {
        switch (c)
        {
        case 'e':
            oflag |= IPC_EXCL;
            break;
        }
    }
    // optind初始值为1 每次调用getopt都自增1
    if(optind != argc - 1)
    {
        fprintf(stderr, "usage: msgreadte [ -e ] <pathname>\n");
        exit(EXIT_FAILURE);
    }

    // 把用户提供的路径名 作为参数传给ftok，导出的键作为msg的key
    msqid = msgget(ftok(argv[optind], 0), oflag);
    assert(msqid != -1);
}