#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void str_echo(int sockfd)
{
  size_t n;
  char buf[BUFSIZ];

  while ((n = read(sockfd, buf, BUFSIZ)) > 0)
  {
    assert(n > 0);
    write(sockfd, buf, n);
  }
}

void sig_chld(int signo)
{
  pid_t pid;
  int stat;
  pid = wait(&stat);
  printf("Child %d terminated\n", pid);
  return;
}

// 并发服务器
int main(int argc, char *argv[])
{
  if (argc <= 2)
  {
    printf("usage:%s ip_address port_number\n", argv[0]);
    return 1;
  }
  pid_t child;
  int listenfd, connfd;
  socklen_t clientLen;
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  struct sockaddr_in clientAddr, serverAddr;

  bzero(&serverAddr, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &serverAddr.sin_addr);
  serverAddr.sin_port = htons(port);

  listenfd = socket(PF_INET, SOCK_STREAM, 0);
  assert(listenfd >= 0);

  int ret = bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  assert(ret != -1);

  ret = listen(listenfd, 5);
  assert(ret != -1);
  signal(SIGCHLD, sig_chld);
  for (;;)
  {
    clientLen = sizeof(clientAddr);
    connfd = accept(listenfd, (sockaddr *)&clientAddr, &clientLen);
    assert(connfd >= 0);
    if ((child = fork()) == 0) // child
    {
      close(listenfd);  // 子进程关闭监听套接字
      str_echo(connfd); // 处理客户
      exit(0);
    }
    close(connfd); // 父进程关闭已连接套接字
  }
  return 0;
}