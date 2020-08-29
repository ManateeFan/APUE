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
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>

constexpr int MAX_EVENT_NUMBER = 1024;
static int pipefd[2];

/* 给fd设置非阻塞 */
int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
/* 将fd添加到epoll监听事件表中 */
void addFd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

/* 
可重入（Reentrant）函数可以由多于一个线程并发使用，而不必担心数据错误。
可重入函数可以在任意时刻被中断，稍后再继续运行，不会丢失数据。
*/

/* 信号处理函数 */
void sig_handler(int sig)
{
    /* 保留原来的errno，在函数最后恢复，保证函数的可重入性 */
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0); /* 信号值写入管道，通知主循环 */
    errno = save_errno;
}

/* 设置信号的处理函数 */
void addSig(int sig)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART; /* 设置成重新调用被该信号终止的系统调用 */
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addFd(epollfd, listenfd);

    /* 使用socketpair创建管道，注册pipefd[0]上的可读事件 */
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setNonBlocking(pipefd[1]);
    addFd(epollfd, pipefd[0]);

    /* 设置一些信号处理函数 */
    addSig(SIGHUP);
    addSig(SIGCHLD);
    addSig(SIGTERM);
    addSig(SIGINT);
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            /* 就绪的文件描述符是listenfd，则处理新连接 */
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                addFd(epollfd, connfd);
            }
            /* 就绪的文件描述符是pipefd[0]，则处理信号 */
            else if (sockfd == pipefd[0] && events[i].events & EPOLLIN)
            {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    /* 每个信号值占1字节 */
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGCHLD:
                        case SIGHUP:
                        {
                            continue;
                        }
                        case SIGTERM:
                        case SIGINT:
                        {
                            stop_server = true;
                        }
                        }
                    }
                }
            }
            else
            {
            }
        }
    }

    printf("close fds\n");
    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}
