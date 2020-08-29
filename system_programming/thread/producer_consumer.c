#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void sys_err(const char *s, int err)
{
    fprintf(stderr, "%s %s", s, strerror(err));
    exit(EXIT_FAILURE);
}

//  互斥量初始化
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 条件变量初始化
pthread_cond_t has_pro = PTHREAD_COND_INITIALIZER;

// product
int val = 0;

void *producer(void *arg)
{
    int s;
    while (1)
    {
        // 上锁
        s = pthread_mutex_lock(&mutex);
        if (s != 0)
        {
            sys_err("pthread_mutex_lock()", s);
        }
        val++;
        printf("producer push %d \n", val);
        // 解锁
        s = pthread_mutex_unlock(&mutex);
        if (s != 0)
        {
            sys_err("pthread_mutex_unlock()", s);
        }

        // 发信号
        s = pthread_cond_signal(&has_pro);
        if (s != 0)
        {
            sys_err("pthread_cond_signal()", s);
        }

        sleep(1);
    }
}

void *consumer(void *arg)
{
    int s;
    while (1)
    {
        // 上锁
        s = pthread_mutex_lock(&mutex);
        if (s != 0)
        {
            sys_err("pthread_mutex_lock()", s);
        }
        while (val == 0)
        {
            pthread_cond_wait(&has_pro, &mutex);
        }
        val--;
        printf("consumer %d  take %d \n",pthread_self(), val + 1);
        // 解锁
        s = pthread_mutex_unlock(&mutex);
        if (s != 0)
        {
            sys_err("pthread_mutex_unlock()", s);
        }

        sleep(3);
    }
}
int main()
{
    pthread_t p, c1, c2;
    int s;

    s = pthread_create(&p, NULL, producer, NULL);
    if (s != 0)
    {
        sys_err("pthread_create()", s);
    }

    // 多消费者
    s = pthread_create(&c1, NULL, consumer, NULL);
    if (s != 0)
    {
        sys_err("pthread_create()", s);
    }
    s = pthread_create(&c2, NULL, consumer, NULL);
    if (s != 0)
    {
        sys_err("pthread_create()", s);
    }

    s = pthread_join(p, NULL);
    if (s != 0)
    {
        sys_err("pthread_join()", s);
    }
    pthread_join(c1, NULL);
    if (s != 0)
    {
        sys_err("pthread_join()", s);
    }
    pthread_join(c2, NULL);
    if (s != 0)
    {
        sys_err("pthread_join()", s);
    }
    exit(EXIT_SUCCESS);
}