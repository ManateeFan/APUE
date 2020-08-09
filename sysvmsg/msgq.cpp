#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

int main()
{
    int msqid;
    msqid = msgget(1119, 0); // 0 打开已存在的msq
    // msgid = msgget(1119, 0666 | IPC_CREAT ｜ IPC_EXCL); //  msqid队列存在，则创建失败
    // msqid = msgget(IPC_PRIVATE, 0666);  // 每次调用都创建新的的消息队列，flag无需设置
    assert(msqid != 0);
    printf("msgget success\n");

    __msqid_ds_new ds;
    msgctl(msqid, IPC_STAT, &ds);

    printf("qnum = %lu\n", ds.msg_qnum);
    printf("mode = %o\n", ds.msg_perm.mode);
    printf("bytes = %ld\n", ds.msg_cbytes);

    // // change mode
    // sscanf("600", "%ho", &ds.msg_perm.mode);
    // msgctl(msqid, IPC_SET, &ds);
    // msgctl(msqid, IPC_STAT, &ds);
    // printf("mode = %o\n", ds.msg_perm.mode);
    // msgctl(msqid, IPC_RMID, NULL);
    printf("rm success\n");
}