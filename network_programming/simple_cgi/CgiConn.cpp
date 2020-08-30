#include "CgiConn.h"
int CgiConn::m_epollfd = -1;
void CgiConn::Init(int epollfd, int sockfd, const struct sockaddr_in &client_addr)
{
    m_epollfd = epollfd;
    m_sockfd = sockfd;
    m_address = client_addr;
    memset(m_buf, '\0', BUFFER_SIZE);
    m_read_idx = 0;
}

void CgiConn::Process()
{
    int idx = 0;
    int ret = -1;

    while (true)
    {
        idx = m_read_idx;
        ret = recv(m_sockfd, m_buf + idx, BUFFER_SIZE - 1 - idx, 0);

        if (ret < 0)
        {
            if (errno != EAGAIN)
            {
                RemoveFd(m_epollfd, m_sockfd); // 读错误，关闭客户连接
            }
            break; // 暂时无数据可读，EAGAIN，就退出循环
        }

        else if (ret == 0) // 客户关闭连接
        {
            RemoveFd(m_epollfd, m_sockfd);
            break;
        }
        else
        {
            m_read_idx += ret; // 更新到已读数据的下一个位置
            std::cout << "user content is " << m_buf << std::endl;

            for (; idx < m_read_idx; ++idx) // 遇到 \r\n，就开始处理客户请求
            {
                if (idx >= 1 && m_buf[idx - 1] == '\r' && m_buf[idx] == '\n')
                {
                    break;
                }
            }

            if (idx == m_read_idx) // 不是 \r\n，就需要读更多数据
            {
                continue;
            }

            m_buf[idx - 1] = '\0'; // 是 \r\n，给数据末尾加\0

            std::string file_name(m_buf);
            if (access(file_name.c_str(), F_OK) == -1) // 查找cgi程序是否存在
            {
                RemoveFd(m_epollfd, m_sockfd); // 不存在，就关闭连接
                break;
            }

            ret = fork(); // 创建子进程来执行cgi程序
            if (ret == -1)
            {
                RemoveFd(m_epollfd, m_sockfd);
                break;
            }
            else if (ret > 0)
            {
                RemoveFd(m_epollfd, m_sockfd); // 父进程关闭客户连接
                break;
            }
            else
            {
                close(STDOUT_FILENO);
                dup(m_sockfd);
                execl(file_name.c_str(), file_name.c_str(), nullptr);
                exit(EXIT_SUCCESS);
            }
        }
    }
}
