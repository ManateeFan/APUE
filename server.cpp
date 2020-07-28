#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

constexpr int USER_LIMIT = 5;   /* 最大用户数量 */
constexpr int BUFFER_SIZE = 64; /* 读缓冲区大小 */
constexpr int FD_LIMIT = 65535; /* 文件描述符数量限制 */
/* 客户数据：客户socket地址、待写到客户端的数据的位置、从客户端读入的数据 */
struct client_data
{
    sockaddr_in address;
    char *write_buf = nullptr;
    char buf[BUFFER_SIZE];
};

/* 设置文件描述符非阻塞 */
int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char *argv[])
{
    /* 创建守护进程 */
    daemon(0, 1);

    if (argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    /* 根据命令行参数，获取ip和端口号 */
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    /* 设置socket地址结构体 */
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    /* 地址族 */
    address.sin_family = AF_INET;
    /* ip本地字节序转换为网络字节序 */
    inet_pton(AF_INET, ip, &address.sin_addr);
    /* 端口本地字节序转换为网络字节序 */
    address.sin_port = htons(port);
    
    /* 创建监听socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    /* 命名socket */
    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);
    /* 监听socket */
    ret = listen(listenfd, 5);
    assert(ret != -1);


    // if(!fork())
    // {
    //     exit(0);
    // }
    // /* 设置文件权限掩码，当进程创建新文件，文件权限为mode & 777 */
    // umask(0);
    // /* 创建新回话，甩开终端 */
    // pid_t sid = setsid();
    // if(sid < 0)
    // {
    //     return -1;
    // }

    // /* 切换工作目录 */
    // if((chdir("/")) < 0)
    // {
    //     return -1;
    // }

    // /* 关闭标准输入设备、标准输出设备、标准错误输出设备 */
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    // /* 将标准输入、标准输出、标准错误输出重定向到/dev/null文件 */
    // open("/dev/null", O_RDONLY);
    // open("/dev/null", O_RDWR);
    // open("/dev/null", O_RDWR);


    /* users数组，分配FD_LIMIT个client_data对象。每个socket都能获得这样一个对象，socket的值作为索引 */
    client_data *users = new client_data[FD_LIMIT];
    
    /* pollfd文件描述符数组 */
    pollfd fds[USER_LIMIT + 1];
    /* 初始用户为0个 */
    int user_counter = 0;
    /* 文件描述符数组初始化 */
    for (int i = 1; i <= user_counter; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    /* 数组的第一个元素设置为监听socket */
    fds[0].fd = listenfd;
    /* 注册listen socket的读事件和错误事件 */
    fds[0].events = POLLIN | POLLERR;
    /* 实际发生的事件，由内核填充 */
    fds[0].revents = 0;

    while (1)
    {
        ret = poll(fds, user_counter + 1, -1);
        if (ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        /* 遍历fds数组，查询事件 */
        for (int i = 0; i < user_counter + 1; ++i)
        {
            /* 如果为listenfd并且它存在读事件 */
            if (fds[i].fd == listenfd && fds[i].revents & POLLIN)
            {
                /* 客户端socket地址结构体 */
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                /* 从监听队列接受一个连接 */
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                if (connfd < 0)
                {
                    perror("accept error");
                    continue;
                }
                /* 用户数量超出限制，关闭新到的连接 */
                if (user_counter >= USER_LIMIT)
                {
                    const char *info = "too many users\n";
                    close(connfd);
                    continue;
                }
                /* 新的连接，同时修改fds数组和users数组 */
                user_counter++;
                users[connfd].address = client_address;
                setNonBlocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user, now have %d users\n", user_counter);
            }
            /* 监听到错误事件 */
            else if (fds[i].revents & POLLERR)
            {
                printf("get an error form %d\n", fds[i].fd);
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                /* 获取套接字错误选项 */
                if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }
            /* 监听到对方关闭tcp连接 */
            else if (fds[i].revents & POLLRDHUP)
            {
                /* 客户端关闭了连接，则服务器也关闭对应连接，用户数-1 */
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                i--;
                user_counter--;
                printf("a client left\n");
            }
            /* 监听到读事件 */
            else if (fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                /* 读数据到该用户的buf里 */
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                printf("get %d bytes of client data %s form %d \n", ret, users[connfd].buf, connfd);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if (ret == 0)
                {
                }
                else
                {
                    /* 接收到客户数据，并通知其他socket准备写数据 */
                    for (int j = 1; j <= user_counter; ++j)
                    {
                        if (fds[j].fd == connfd)
                            continue;
                        /* 取消监听读事件 */
                        fds[j].events |= ~POLLIN;
                        /* 增加监听写事件 */
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }
            }
            /* 监听到写事件 */
            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if (!users[connfd].write_buf)
                    continue;
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = nullptr;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
