#ifndef TIMER
#define TIMER

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

constexpr int BUFFER_SIZE = 64;

class tw_timer;
/* 绑定socket和定时器 */
struct client_data
{
    struct sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

/* 定时器类 */
class tw_timer
{
public:
    tw_timer(int rot, int ts) : rotation{rot}, time_slot{ts} {}

    int rotation;  /* 记录定时器在时间轮转多少圈后生效 */
    int time_slot; /* 记录定时器属于时间轮上哪个槽 */
    using cb_func = void (*)(client_data *);
    cb_func func;            /* 定时器回调函数 */
    client_data *user_data;  /* 客户数据 */
    tw_timer *next{nullptr}; /* 指向下一个定时器 */
    tw_timer *prev{nullptr}; /* 指向前一个定时器 */
};

/* 时间轮 */
class time_wheel
{
public:
    time_wheel()
    {
        for (int i = 0; i < N; ++i)
        {
            slots[i] = nullptr; /* 初始化每个槽的头节点 */
        }
    }

    ~time_wheel()
    {
        /* 遍历时间轮的每个槽，并销毁其中的定时器 */
        for (int i = 0; i < N; ++i)
        {
            tw_timer *tmp = slots[i];
            while (tmp) /* 头删法 */
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }

    /* 根据定时值timeout创建一个定时器，并插入合适的槽中 */
    tw_timer *add_timer(int timeout)
    {
        if (timeout < 0)
        {
            return nullptr;
        }
        int ticks = 0;
        /* 根据timeout计算它在时间轮指针转动几个滴答后被触发， 并将滴答数存在ticks中。 */
        if (timeout < SI) /* 小于滴答声，扩大为1 */
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI;
        }
        /* 计算待插入的定时器在时间轮转动多少圈后被触发 */
        int rotation = ticks / N;
        /* 计算它该被插入哪个槽中 */
        int ts = (cur_slot + (ticks % N)) % N;
        /* 创建定时器，他在时间轮转动rotation圈后触发，位于第ts个槽上 */
        tw_timer *timer = new tw_timer(rotation, ts);

        /* 如果第ts个槽是空的 */
        if (!slots[ts])
        {
            printf("add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        { /* 头插 */
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }
    /* 删除目标定时器 */
    void del_timer(tw_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        int ts = timer->time_slot;
        if (timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if (slots[ts])
            {
                slots[ts]->prev = nullptr;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if (timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }
    /* SI 时间到后，时间轮指针指向下一个槽 */
    void tick()
    {
        tw_timer *tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while (tmp)
        {
            printf("tick the timer once\n");
            if (tmp->rotation > 0) /* 这一轮不起作用 */
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else
            {
                tmp->func(tmp->user_data);
                if (tmp == slots[cur_slot])
                {
                    printf("delete header in cur_slot\n");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if (slots[cur_slot])
                    {
                        slots[cur_slot]->prev = nullptr;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if (tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer *tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot += ++cur_slot % N; /* 更新时间轮当前槽 */
    }

private:
    /* 时间轮上槽的数量 */
    static constexpr int N = 60;
    /* 每1s时间轮转动一次，即槽指针指向下一个槽 */
    static constexpr int SI = 1;
    /* 时间轮的槽，每个元素指向一个定时器链表，链表无序 */
    tw_timer *slots[N];
    int cur_slot = 0; /* 当前时间轮指针指向的槽 */
};

#endif