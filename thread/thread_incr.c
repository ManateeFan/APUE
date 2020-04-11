#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// 线程以非原子方式访问共享变量 存在的问题

void sys_err(const char* s, int err)
{
    fprintf(stderr,"%s %s", s, strerror(err));
    exit(EXIT_FAILURE);
}

// 共享全局变量
static int glob = 0;

// 线程调用函数
static void*
threadFunc(void* arg)
{
    int loops = *((int*)arg);
    int local, j;

    for(j = 0; j < loops; j++)
    {
        local = glob;
        local++;
        glob = local;
    }
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int loops, s;
    
    loops = 10000000;

    // 两个线程
    s = pthread_create(&t1,NULL,threadFunc,&loops);
    if(s != 0)
    {
        sys_err("pthread_create()", s);
    }

    s = pthread_create(&t2, NULL,threadFunc, &loops);
    if(s != 0)
    {
        sys_err("pthread_create()", s);
    }
    // 线程连接
    s = pthread_join(t1, NULL);
    if(s != 0)
    {
        sys_err("pthread_join()", s);
    }

    s = pthread_join(t2, NULL);
    if(s != 0)
    {
        sys_err("pthread_join()", s);
    }

    printf("glob = %d\n", glob);
    exit(EXIT_SUCCESS);
}