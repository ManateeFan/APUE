#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_t ntid;

void PrintId(const char *s)
{
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);
}

void *ThreadFunc(void *arg)
{
    PrintId("new thread: ");
    return (void *)0;
}

int main()
{
    int err;
    err = pthread_create(&ntid, NULL, ThreadFunc, NULL);
    if (err != 0)
    {
        perror("pthread_create err");
        return 1;
    }
    PrintId("main thread: ");
    sleep(1);
}
