#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0);

ssize_t readn(int fd, void *buf, size_t n)
{
    int nleft = n;
    int nread = 0;
    char *p = (char *)buf;
    while (nleft > 0)
    {
        if ((nread = read(fd, p, nleft)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        else if (nread == 0)
        {
            return n - nleft;
        }
        p += nread;
        nleft -= nread;
    }
    return n;
}

ssize_t writen(int fd, void *buf, size_t n)
{
    int nwrite = 0;
    int nleft = n;
    char *p = (char *)buf;
    while (nleft > 0)
    {
        if ((nwrite = write(fd, p, nleft)) < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        else if (nwrite == 0)
        {
            continue;
        }
        p += nwrite;
        nleft -= nwrite;
    }
    return n;
}

struct Packet
{
    size_t len;
    char buf[BUFSIZ];
};

int main(int argc, char **argv)
{
    // 1. 创建套接字
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // inet_aton("127.0.0.1", &servaddr.sin_addr);

    int on = 1;
    // 确保time_wait状态下同一端口仍可使用
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
    {
        ERR_EXIT("setsockopt");
    }

    // 3. 绑定套接字地址
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof servaddr) < 0)
    {
        ERR_EXIT("bind");
    }
    // 4. 等待连接请求状态
    if (listen(listenfd, SOMAXCONN) < 0)
    {
        ERR_EXIT("listen");
    }
    // 5. 允许连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof peeraddr;

    int connfd;
    while (1)
    {

        if ((connfd = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
        {
            ERR_EXIT("accept");
        }
        printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
        printf("port = %d\n", ntohs(peeraddr.sin_port));

        pid_t pid;
        pid = fork();
        if (pid == -1)
        {
            ERR_EXIT("fork");
        }
        if (pid == 0)
        {
            close(listenfd);
            // 6. 数据交换
            Packet recvPacket;
            while (1)
            {
                memset(&recvPacket, 0, sizeof(recvPacket));
                int ret = readn(connfd, &recvPacket.len, sizeof(recvPacket.len));
                if (ret == -1)
                {
                    ERR_EXIT("readn");
                }
                else if(ret < sizeof(recvPacket.len))
                {
                    printf("client close\n");
                    break;
                }
                int n = ntohl(recvPacket.len);
                ret = readn(connfd, recvPacket.buf, n);
                if(ret == -1)
                {
                    ERR_EXIT("readn");
                }
                else if(ret < n)
                {
                    printf("client close\n");
                    break;
                }
                fputs(recvPacket.buf, stdout);
                writen(connfd, &recvPacket, n + sizeof(recvPacket.len));
            }

            // 7. 断开连接
            close(connfd);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(connfd);
        }
    }
    close(listenfd);
    return 0;
}
