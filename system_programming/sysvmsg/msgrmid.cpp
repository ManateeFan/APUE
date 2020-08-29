#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int msqid = msgget(ftok(argv[1], 0), 0);
    msgctl(msqid, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}