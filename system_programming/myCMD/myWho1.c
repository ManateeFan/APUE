#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <utmp.h>

struct myUtmp
{
  char ut_line[UT_LINESIZE];
  char ut_name[UT_NAMESIZE];
  char ut_host[UT_HOSTSIZE];
  long ut_time;
};

void showInfo(struct myUtmp* tmp)
{
  printf("%-8.8s", tmp->ut_name);
  printf("  ");
  printf("%-8.8s", tmp->ut_line);
  printf("%10ld", tmp->ut_time);
  printf("  ");
  printf("(%s)", tmp->ut_host);
  printf("\n");
}

int main()
{
  // utmp 结构体
  struct myUtmp tmp;

  // 结构体大小
  int utmpLen = sizeof(tmp);
  int fd;

  // 若打开失败
  // _PATH_UTMP 在mac不适用  mac为/var/run/utmpx文件
  if ((fd = open("/var/run/utmpx", O_RDONLY, 0600)) == -1)
  {
    perror("open()");
    return 1;
  }

  // 每次读入正确大小并写入myUtmp中
  while (read(fd, &tmp, utmpLen) == utmpLen)
  {
    // 打印用户信息
    showInfo(&tmp);
  }

  // 关闭文件描述符
  close(fd);
  return 0;
}