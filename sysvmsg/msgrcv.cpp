#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

struct msgbuf
{
    long mtype;
    char mtext[BUFSIZ];
};

int main(int argc, char* argv[])
{
    char c = 0;
    int flag = 0;
    int type = 0;

    while((c = getopt(argc, argv, "nt:")) != -1)
    {
        switch (c)
        {
        case 'n':
            flag |= IPC_NOWAIT;
            break;
        case 't':
            type = atoi(optarg);
            break;
        }
    }
    if(optind != argc - 1)
    {
        fprintf(stderr, "usage: %s [ -n ] [-t type] <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int msqid = msgget(ftok(argv[optind], 0), 0664);
    assert(msqid != -1);

    msgbuf* buf = new msgbuf;
    int n = msgrcv(msqid, buf, BUFSIZ, type, flag);
    printf("read %d bytes, type = %ld\n", n, buf->mtype);
    delete buf;
    exit(0);
}