#ifndef CGICONN_H
#define CGICONN_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "process_pool.hpp"

class CgiConn
{
private:
    static constexpr int BUFFER_SIZE = 1024; // read buffer size
    static int m_epollfd;
    int m_sockfd;
    struct sockaddr_in m_address;
    char m_buf[BUFFER_SIZE];
    int m_read_idx; // 读缓冲中已经读入的客户数据的最后一个字节的下一个位置

public:
    CgiConn() = default;
    ~CgiConn() = default;

    void Init(int epollfd, int sockfd, const sockaddr_in &client_addr); // 初始化客户连接，清空读缓冲区
    void Process();                                                     // 处理客户请求
};

#endif