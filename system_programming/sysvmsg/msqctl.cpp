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
    char mtext[1];
};

int main(int argc, char *argv[])
{
    int msqid;
    msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    assert(msqid != -1);

    msgbuf buf;
    buf.mtype = 1;
    buf.mtext[0] = '1';
    int ret = msgsnd(msqid, &buf.mtype, sizeof(buf.mtext), 0);
    assert(ret == 0);

    msqid_ds info;
    ret = msgctl(msqid, IPC_STAT, &info);
    assert(ret == 0);

    printf("read-write : %03o, cbytes = %lu, qnum = %lu, qbytes = %lu\n", info.msg_perm.mode & 0777, info.msg_cbytes,
           info.msg_qnum, info.msg_qbytes);

    system("ipcs -q");

    msgctl(msqid, IPC_RMID, NULL);
}