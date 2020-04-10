#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// 线程调用函数
static void *
threadFunc(void *arg)
{
    char *s = (char *)arg;
    printf("%s", s);
    return (void *)strlen(s);
}

int main()
{
    pthread_t t1;
    void *ret;
    int s;

    s = pthread_create(&t1, NULL, threadFunc, "Hello World\n");
    if (s == -1)
    {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }
    printf("Message from main()\n");

    // 线程连接函数 用于接收t1的返回值
    s = pthread_join(t1, &ret);
    if (s != 0)
    {
        perror("pthread_join()");
        exit(EXIT_FAILURE);
    }
    printf("Thread returned %ld\n", (long)ret);

    exit(EXIT_SUCCESS);
}