#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool stop = false;
/* SIGTERM信号处理函数*/
static void handle_term(int sig)
{
    stop = true;
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, handle_term);
    if (argc < 3)
    { // 接收 ip port backlog
        printf("usage:%s ip_address port_number backlog\n", argv[0]);
        return 1;
    }

    // ip + port
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // 监听队列最大长度
    int backlog = atoi(argv[3]);

    // socket f
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // 创建IPv4 socket地址
    struct sockaddr_in address;

    // 置0
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;

    // port转为网络字节序
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    // 命名绑定
    int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    // 监听
    ret = listen(sock, backlog);
    assert(ret != -1);

    // 循环等待连接，知道SIGTERM信号将它中断
    while (!stop)
    {
        sleep(1);
    }

    // 关闭socket
    close(sock);
    return 0;
}
