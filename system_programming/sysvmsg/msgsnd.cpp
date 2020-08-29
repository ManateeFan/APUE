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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <pathname> <type>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    int type = atoi(argv[2]);

    int msqid;
    msqid = msgget(ftok(argv[1], 0), 0666 | IPC_CREAT); // 0 打开已存在的msq
    assert(msqid != 0);
    printf("msgget success\n");

    msgbuf* buf = new msgbuf;
    buf->mtype = type;
    
    int ret = msgsnd(msqid, buf, BUFSIZ, 0);
    if (ret < 0)
    {
        fprintf(stderr, "msgsnd error\n");
        exit(EXIT_SUCCESS);
    }

    delete buf;
    exit(EXIT_SUCCESS);
}