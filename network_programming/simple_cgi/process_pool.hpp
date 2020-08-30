#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <memory>
#include <cassert>

class Process
{
public:
    Process() : m_pid(-1)
    {}
    pid_t m_pid;    // 父进程中为子进程id，子进程中为-1
    int m_pidfd[2]; // 父子进程的管道
};

template <typename T>
class ProcessPool
{
private:
    ProcessPool(int listenfd, int process_number = 8);
    void SetupEpollAndSigPipe();
    void RunParent();
    void RunChild();

public:
    static ProcessPool<T> *Create(int listenfd, int process_number = 8)
    {
        if (!m_instance)
        {
            m_instance = new ProcessPool<T>(listenfd, process_number);
        }
        return m_instance;
    }
    void Run();
    ~ProcessPool() = default;

private:
    static constexpr int MAX_PROCESS_NUMBER = 16; // 进程池最大进程数量
    static constexpr int USER_PROCESS = 65536;    // 每个子进程最多处理的客户连接
    static constexpr int MAX_EVENT_NUMBER = 1000; // epoll 最多能处理的事件个数
    int m_process_number;                         // 进程池中的进程总数
    int m_idx;                                    // 子进程在池中的序号
    int m_epollfd;                                // 每个进程拥有的epollfd
    int m_listenfd;                               // 每个进程的listenfd
    bool m_stop;                                  // 决定子进程是否停止运行
    std::vector<Process> m_sub_process;           // 所有子进程的信息
    static ProcessPool<T> *m_instance;            // 进程池静态实例
};

template <typename T>
ProcessPool<T> *ProcessPool<T>::m_instance = nullptr;

static int SetNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

static void AddFd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET; // 边沿触发
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    SetNonBlocking(fd);
}

static void RemoveFd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

static void AddSig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

template <typename T>
ProcessPool<T>::ProcessPool(int listenfd, int process_number)
    : m_listenfd(listenfd), m_process_number(process_number), m_idx(-1), m_stop(false)
{
    assert((process_number > 0) && (process_number <= MAX_PROCESS_NUMBER));

    for (int i = 0; i < process_number; ++i)
    {
        m_sub_process.push_back(Process());
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process.at(i).m_pidfd);
        assert(ret == 0);

        m_sub_process.at(i).m_pid = fork();
        assert(m_sub_process.at(i).m_pid >= 0);
        if (m_sub_process.at(i).m_pid > 0)
        {
            close(m_sub_process.at(i).m_pidfd[1]);
            continue; // 父进程continue创建子进程
        }
        else
        {
            close(m_sub_process.at(i).m_pidfd[0]);
            m_idx = i;
            break; // 子进程break跳出循环
        }
    }
}

static int sig_pipefd[2]; // 处理信号的管道, 1为写端，0为读端

static void SigHandler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

template <typename T>
void ProcessPool<T>::SetupEpollAndSigPipe()
{
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    SetNonBlocking(sig_pipefd[1]);   // 写端非阻塞
    AddFd(m_epollfd, sig_pipefd[0]); // 读端监听读事件

    AddSig(SIGCHLD, SigHandler); // 子进程状态改变，就向写端写入信号
    AddSig(SIGTERM, SigHandler);
    AddSig(SIGINT, SigHandler);
    AddSig(SIGPIPE, SIG_IGN);
}

template <typename T>
void ProcessPool<T>::Run()
{
    if (m_idx != -1) // 父进程中m_idx为-1
    {
        RunChild();
    }
    else
    {
        RunParent();
    }
}

template <typename T>
void ProcessPool<T>::RunChild()
{
    SetupEpollAndSigPipe();                          // 创建 该子进程自己的epoll事件监听表 和信号管道
                                                     // 子进程通过监听pipefd，来接受新客户连接
    int pipefd = m_sub_process.at(m_idx).m_pidfd[1]; // 构造子进程时已经关闭了0端
    AddFd(m_epollfd, pipefd); // 父进程通过pipefd[1]通知改子进程accept新的客户连接

    std::vector<epoll_event> events(MAX_EVENT_NUMBER, epoll_event());
    std::vector<T> users(USER_PROCESS, T());

    int number = 0;
    int ret = -1;

    while (!m_stop)
    {
        number = epoll_wait(m_epollfd, &events.front(), MAX_EVENT_NUMBER, -1);
        if (number < 0 && (errno != EINTR)) // 不属于慢系统调用被中断
        {
            std::cout << "epoll failure" << std::endl;
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            if ((sockfd == pipefd) &&
                (events[i].events & EPOLLIN)) // 父进程通过pipefd[0]端发来信息,表示有新客户连接到来
            {
                int client = 0;
                ret = recv(sockfd, (char *)&client, sizeof(client), 0);

                if (((ret < 0) && (errno != EAGAIN)) || ret == 0) // 若出错
                {
                    continue;
                }
                else
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addr_length;
                    int connfd =
                        accept(m_listenfd, (struct sockaddr *)&client_address, &client_addr_length); // 接受新客户连接

                    if (connfd < 0)
                    {
                        std::cout << "errno is : %d" << errno << std::endl;
                        continue;
                    }

                    AddFd(m_epollfd, connfd);
                    users[connfd].Init(
                        m_epollfd, connfd, client_address); // 模版T类需实现Init方法，使用connfd索引用户，提高效率
                }
            }

            else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN)) // 父进程向信号管道发来信号信息
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if (ret <= 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                            pid_t pid;
                            int status;
                            while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
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
            else if (events[i].events & EPOLLIN) // 其他fd上的读事件，即客户请求到来，使用客户的Process方法处理
            {
                users[sockfd].Process();
            }
            else
            {
                continue;
            }
        }
    }

    close(pipefd);    // 该子进程任务执行完毕，关闭pipefd
    close(m_epollfd); // 关闭epollfd
}

template <typename T>
void ProcessPool<T>::RunParent()
{
    SetupEpollAndSigPipe(); // 创建 父进程自己的epoll事件监听表 和信号管道

    AddFd(m_epollfd, m_listenfd); // 父进程监听listenfd

    std::vector<epoll_event> events(MAX_EVENT_NUMBER, epoll_event());
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while (!m_stop)
    {
        number = epoll_wait(m_epollfd, &events.front(), MAX_EVENT_NUMBER, -1);

        if ((number < 0) && (errno != EINTR))
        {
            std::cout << "epoll failure" << std::endl;
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            if (sockfd == m_listenfd) // 有客户新连接到来, 将其分配给一个子进程处理
            {
                int j = sub_process_counter;
                do
                {
                    if (m_sub_process[j].m_pid != -1) // 找到产生的子进程
                    {
                        break;
                    }
                    j = (j + 1) % m_process_number;
                } while (j != sub_process_counter);

                if (m_sub_process[j].m_pid == -1)
                {
                    m_stop = true;
                    break;
                }

                sub_process_counter = (j + 1) % m_process_number;
                send(m_sub_process[j].m_pidfd[0], (char *)&new_conn, sizeof(new_conn), 0);
                std::cout << "send request to child" << j << std::endl;
            }
            else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN)) // 父进程收到子进程发来的信号
            {
                int sig;
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);

                if (ret <= 0)
                {
                    continue;
                }
                else
                {
                    for (int j = 0; j < ret; ++j)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                        {
                            pid_t pid;
                            int status;
                            while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
                            {
                                for (int k = 0; k < m_process_number; ++k)
                                {
                                    if (m_sub_process[k].m_pid =
                                            pid) // 第k个子进程退出了，就关闭对应的通信管道，并设置m_pid为-1，标记为退出
                                    {
                                        std::cout << "child %d join" << k << std::endl;
                                        close(m_sub_process[k].m_pidfd[0]);
                                        m_sub_process[i].m_pid = -1;
                                    }
                                }
                            }

                            m_stop = true; // 父进程先设置为退出状态
                            for (int k = 0; k < m_process_number; ++k)
                            {
                                if (m_sub_process[k].m_pid != -1)
                                {
                                    m_stop = false; // 若有未退出的子进程，则父进程就不退出了
                                }
                            }
                            break;
                        }
                        case SIGTERM:
                        case SIGINT: // 父进程接到终止信号，则杀死所有子进程
                        {
                            std::cout << "kill all the child now" << std::endl;
                            for (int j = 0; j < m_process_number; ++j)
                            {
                                int pid = m_sub_process[j].m_pid;
                                if (pid != -1)
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