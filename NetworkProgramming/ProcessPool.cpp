#ifndef PROCESSPOOL_H
#define PROCESSPOOL_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// 子进程类
class Process
{
public:
    Process()
        : m_pid(-1)
    {
    }
    pid_t m_pid;     // 子进程PID
    int m_pipefd[2]; // 父子进程通信的管道
};

// 进程池类
template <typename T>
class ProcessPool
{
private:
    ProcessPool(
        int listenfd,
        int process_number = 8); // 只通过create静态函数创建ProcessPool实例
    void setup_sig_pipe();
    void run_parent();
    void run_child();

public:
    ProcessPool(int listenfd, int process_number = 8);
    ~ProcessPool() { delete[] m_sub_process; }
    static ProcessPool *create(
        int listenfd,
        int process_number = 8) // 单体模式，保证最多创建一个实例，为了正确处理信号
    {
        if (!m_instance)
        {
            m_instance = new ProcessPool(listenfd, process_number);
        }
        return m_instance;
    }
    void run(); // 启动线程池
private:
    static constexpr int MAX_PROCESS_NUMBER = 16;  // 进程池最大子进程数量
    static constexpr int USER_PER_PROCESS = 65536; // 每个子进程最多处理的客户数量
    static constexpr int MAX_EVENT_NUMBER = 10000; // epoll最多能处理的事件数量
    int m_process_number;                          // 进程池中进程总数
    int m_idx;                                     // 子进程在池中的序号，0.1.2...
    int m_epollfd;                                 // 每个进程的epoll内核事件表
    int m_listenfd;                                // 监听socket
    bool m_stop;                                   // 子进程是否停止运行
    Process *m_sub_process;                        // 所有子进程的描述信息
    static ProcessPool *m_instance = nullptr;      // 进程池静态实例
};

static int sig_pipefd[2]; // 处理信号的管道,实现统一事件源

static int setNonBlocking(int fd) // 设置文件描述符非阻塞
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

static void addfd(int epollfd, int fd) // 向epoll事件表添加fd的读事件，设置ET模式
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

static void removefd(int epollfd, int fd) // 从epoll事件表中删除fd上的所有注册的事件
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

auto sig_handler = [](int sig) { //  主线程的信号处理函数
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char *)&msg, 1, 0); // 将收到的信号通过信号处理管道发给子进程
    errno = save_errno;
};

static void addsig(int sig, void(handler)(int), bool restart = true) // 设置某个信号的信号处理函数
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART; // 使被信号打断的系统调用重新发起
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

template <typename T>
ProcessPool<T>::ProcessPool(int listenfd, int process_number = 8) : m_listenfd(listenfd), m_process_number(process_number), m_idx(-1), m_stop(false)
{
    assert((process_number > 0) && (process_number <= MAX_PROCESS_NUMBER));

    m_sub_process = new Process[process_number]; // 子进程数组
    assert(m_sub_process);

    for(int i = 0; i < process_number; ++i)
    {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);   // 创建父子进程间的双向管道
        assert(ret == 0);

        m_sub_process[i].m_pid = fork();
        assert(m_sub_process[i].m_pid >= 0)
        if(m_sub_process[i].m_pid > 0)
        {
            close(m_sub_process[i].m_pipefd[1])
        }
        else
        {
            close(m_sub_process[i].m_pipefd[0]);
            m_idx = i;                              // 子进程在池中编号
        }
    }
}

template<typename T>
void ProcessPool<T>::setup_sig_pipe()               // 统一事件源
{
    m_epollfd = epoll_create(5);                    // epoll事件监听表
    assert(m_epollfd != -1);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    setNonBlocking(sig_pipefd[1]);
    addfd(m_epollfd, sig_pipefd[0]);

    addsig(SIGCHLD, sig_handler);                   // 设置父进程的信号处理函数
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, SIG_IGN);
}

template<typename T>
void ProcessPool<T>::run()
{
    if(m_idx != -1)                                 // 运行子进程
    {
        run_child();
        return;
    }
    run_parent();                                   // 运行父进程
}

template<typename T>
void ProcessPool<T>::run_child()
{
    setup_sig_pipe();
    int pipfd = m_sub_process[m_idx].m_pipefd[1];   // 子进程通过自己在池中的idx找到与父进程通信的管道
    addfd(m_epollfd, pipefd);                       // 子进程监听pipefd上父进程发来的通知，从而accept新连接

    epoll_event events[USER_PER_PROCESS];
    T* users = new T[USER_PER_PROCESS];
    assert(users);
    int number = 0;
    int ret = -1;
    while(!m_stop)
    {
        number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno != EINTR)
        {
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if((sockfd== pipefd) && (events[i].events & EPOLLIN))   // 父进程发来通知
            {
                int client = 0;
                ret = recv(sockfd, (char*)&client, sizeof(client), 0);
                if(((ret < 0) && (errno != EAGAIN)) || ret == 0)
                {
                    continue;
                }
                else
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addrlength = sizeof(client_address);
                    int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                    if(connfd < 0)
                    {
                        printf("errno is : %d\n", errno);
                        continue;
                    }
                    addfd(m_epollfd, connfd);

                    users[connfd].init(m_epollfd, connfd, client_address);  // T 需要实现init方法，来初始化客户连接
                }
            }
            else if((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))  // 处理子进程接受到的信号
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if(ret < 0)
                {
                    continue;
                }
                else
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                            pid_t pid;
                            int stat;
                            while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                            {
                                continue;
                            }
                            break;
                        case SIGTERM:
                        case SIGINT:
                        {
                            m_stop = true;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                } 
            }
            else if(events[i].events & EPOLLIN)
            {
                users[sockfd].process();
            }
            {
                continue;
            }
            
        }
    }
    delete[] users;
    users = nullptr;
    close(pipefd);
    close(m_epollfd);
}

template<typename T>
void ProcessPool<T>::run_parent()
{
    setup_sig_pipe();

    addfd(m_epollfd, m_listenfd);
    epoll_event events[MAX_EVENT_NUMBER];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while (!m_stop)
    {
        number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < number; ++i)
        {
            int sockfd = event[i].data.fd;
            if(sockfd == m_listenfd)
            {
                int j = sub_process_counter;
                do
                {
                    if(m_sub_process[j].m_pid != -1)
                    {
                        break;
                    }
                    j = (j + 1) % m_process_number;
                } while (j!=sub_process_counter);

                if(m_sub_process[j].m_pid == -1)
                {
                    m_stop = true;
                    break;
                }
                sub_process_counter = (j + 1) % m_process_number;
                send(m_sub_process[j].m_pipefd[0], (char*)&new_conn, sizeof(new_conn), 0);
                printf("send request to child %d\n", i);
            }
            else if(sockfd == sig_pipefd[0] && events[i].events & EPOLLIN)      // 处理父进程接收到的信号
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if(ret < 0)
                {
                    continue;
                }
                else
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                            pid_t pid;
                            int stat;
                            while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                            {
                                for(int i = 0; i < m_process_number; ++i)
                                {
                                    if(m_sub_process[i].m_pid == pid)   // 第i个子进程退出
                                    {
                                        printf("child %d join\n", i);
                                        close(m_sub_process[i].m_pipefd[0]);
                                        m_sub_process[i].m_pid = -1;
                                    }
                                }
                            }
                            m_stop = true;
                            for(int i = 0; i < m_process_number; ++i)
                            {
                                if(m_sub_process[i].m_pid != -1)
                                {
                                    m_stop = false;
                                }
                            }
                            break;
                        case SIGTERM:
                        case SIGINT:
                        {
                            printf("kill all the child now\n");
                            for(int i = 0; i < m_process_number; ++i)
                            {
                                int pid = m_sub_process[i].m_pid;
                                if(pid != -1)
                                {
                                    kill(pid, SIGTERM);
                                }
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                } 
            }
            else
            {
                continue;
            }
            
        }
    }
    close(m_epollfd);
}
#endif