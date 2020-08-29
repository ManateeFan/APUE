#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>

#define BUFFER_SIZE 1024
/* 定义两种HTTP状态码和状态信息 */
static const char *status_line[2] = {"200 OK", "500 Internal server error"};

int main(int argc, char *argv[])
{

    if (argc <= 3)
    {
        printf("usage:%s ip_address port_number filename\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    /* http请求的目标文件 */
    const char *file_name = argv[3];

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr *)&address, &client_addrlength);
    if (connfd < 0)
    {
        perror("accept error");
    }
    else
    {
        /* 保存http应答的状态行、头部字段和一个空行的缓存区 */
        char header_buf[BUFFER_SIZE];
        memset(header_buf, '\0', BUFFER_SIZE);
        /* 存放目标文件内容的应用程序缓存 */
        char *file_buf;
        /* 获取目标文件属性，判断文件是否为目录，文件大小等 */
        struct stat file_stat;
        /* 目标文件是否有效 */
        bool valid = true;
        /* header_buf已用字节数 */
        int len = 0;
        if (stat(file_name, &file_stat) < 0) /* 目标文件不存在 */
        {
            valid = false;
        }
        else
        {
            if (S_ISDIR(file_stat.st_mode)) /* 目标文件为目录 */
            {
                valid = false;
            }
            else if (file_stat.st_mode & S_IROTH)
            {
                /* 为file_buf动态分配内存，大小为file_stat.st_size + 1，再将文件内容读入缓存区中 */
                int fd = open(file_name, O_RDONLY);
                file_buf = (char *)malloc(file_stat.st_size + 1);
                memset(file_buf, '\0', file_stat.st_size + 1);
                if (read(fd, file_buf, file_stat.st_size) < 0)
                {
                    valid = false;
                }
            }
            else
            {
                valid = false;
            }
        }
        /* 目标文件有效，发送http应答 */
        if (valid)
        {
            /* 将http应答状态行、头部字段和空行加入head_buf中 */
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
            len += ret;
            ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %lld\r\n", file_stat.st_size);
            len += ret;
            ret = snprintf(header_buf, BUFFER_SIZE - 1 - len, "%s", "\r\n");
            /* 利用writev将head_buf和file_buf的内容一块写出 */
            struct iovec iv[2];
            iv[0].iov_base = header_buf;
            iv[0].iov_len = strlen(header_buf);
            iv[1].iov_base = file_buf;
            iv[1].iov_len = file_stat.st_size;
            ret = writev(connfd, iv, 2);
        }
        else
        {
            ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
            len += ret;
            ret = snprintf(header_buf, BUFFER_SIZE - 1 - len, "%s", "\r\n");
            send(connfd, header_buf, strlen(header_buf), 0);
        }
        close(connfd);
        free(file_buf);
    }

    close(sock);
    return 0;
}