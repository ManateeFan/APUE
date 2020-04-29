#ifndef TIMER
#define TIMER

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

constexpr int BUFFER_SIZE = 64;
class util_timer;

/* 用户数据结构：socket地址、socket文件描述符、读缓存和定时器 */
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

/* 定时器类 */
class util_timer
{
public:
    time_t expire;                           /* 任务的超时事件，绝对 */
    using cb_func = void (*)(client_data *); /* 任务回调函数 */
    cb_func func;
    client_data *user_data;
    util_timer *prev = nullptr;
    util_timer *next = nullptr;
};

/* 定时器双向链表，升序 */
class sort_timer_list
{
private:
    util_timer *head = nullptr;
    util_timer *tail = nullptr;
    /* 将定时器timer插到节点list_head之后的链表中 */
    void add_timer(util_timer *timer, util_timer *list_head)
    {
        util_timer *prev = list_head;
        util_timer *tmp = prev->next;

        while (tmp)
        {
            if (timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }

        if (!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = nullptr;
            tail = timer;
        }
    }

public:
    ~sort_timer_list()
    {
        util_timer *tmp = head;
        while (tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    /* 将目标定时器timer插入链表中 */
    void add_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        if (!head)
        {
            head = tail = timer;
            return;
        }
        /* timer超时时间小于链表中所有定时器的超时时间，则插入头部 */
        if (timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }
    /* 某定时器超时时间延长，向链表尾部调整 */
    void addjust_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        util_timer *tmp = timer->next;
        /* 如果该定时器在尾部，或它的下一个定时器的超时值大于它，则不需要调整 */
        if (!tmp || tmp->expire > timer->expire)
        {
            return;
        }
        /* 如果它是头节点，就取出并重新插入 */
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        }
        /* 不是头节点,就插入它后面部分 */
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }
    /* 移除timer */
    void del_timer(util_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        if (timer == head && timer == tail)
        {
            delete timer;
            head = tail = nullptr;
            return;
        }
        /* 链表至少有两个定时器，且timer为该链表的头节点 */
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        /* 链表至少有两个定时器，且timer为该链表的尾节点 */
        if (timer == tail)
        {
            tail = timer->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }
        /* timer位于链表中间 */
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }
    /* SIGALRM信号每次触发，就在信号处理函数中执行该函数，以处理到期任务 */
    void tick()
    {
        if (!head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(nullptr);
        util_timer *tmp = head;
        /* 从链表头部开始一次处理每个定时器， 知道遇到一个没有到期的定时器 */
        while (tmp)
        {
            if (cur < tmp->expire)
            {
                break;
            }
            /* 调用定时器回调函数，执行定时任务 */
            tmp->func(tmp->user_data);
            /* 执行完后，删除该定时器，并重置链表头节点 */
            head = tmp->next;
            if (head)
            {
                head->prev = nullptr;
            }
            delete tmp;
            tmp = head;
        }
    }
};

#endif