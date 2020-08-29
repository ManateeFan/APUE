#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int i, j, maxi, listenfd, sockfd, connfd;
  int nready;  // poll返回值，就绪fd个数
  ssize_t n;

  char buf[BUFSIZ], str[INET_ADDRSTRLEN];
  socklen_t clilen;
  struct pollfd client[1024];
  struct sockaddr_in cliaddr, servaddr;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  //端口复用
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(11111);
  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd, 128);

  client[0].fd = listenfd;
  client[0].events = POLLIN;  // listenfd监听读时间

  for (int i = 1; i < 1024; i++) {
    client[i].fd = -1;  // -1 初始化
  }

  maxi = 0;

  while (1) {
    nready = poll(client, maxi + 1, -1);
    int i;
    if (client[0].revents & POLLIN) {
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
      printf("Received from %s at PORT %d\n",
             inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, str, sizeof(str)),
             ntohs(cliaddr.sin_port));
      for (i = 1; i < 1024; i++) {
        if (client[i].fd < 0) {
          client[i].fd = connfd;
          if (i > maxi) maxi = i;
          break;
        }
      }

      if (i == 1024) {
        printf("Too many clients\n");
        exit(1);
      }

      client[i].events = POLLIN;
      if (--nready <= 0) continue;
    }

    for (i = 1; i < maxi; i++) {  // 检测客户socketfd
      if ((sockfd = client[i].fd) < 0) continue;

      if (client[i].revents & POLLIN) {
        if ((n = read(sockfd, buf, sizeof(buf))) < 0) {
          if (errno == ECONNRESET) {
            printf("Close client error");
            close(sockfd);
            client[i].fd = -1;
          } else {
            exit(1);
          }
        } else if (n == 0) {
          printf("Close by client");
          close(sockfd);
          client[i].fd = -1;
        } else {
          for (j = 0; j < n; j++) buf[j] = toupper(buf[j]);
          write(sockfd, buf, n);
        }
        if (--nready <= 0) break;
      }
    }
  }

  exit(0);
}