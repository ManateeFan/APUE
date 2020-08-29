#include <stdio.h>
#include <stdlib.h>

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
  FILE* fp;

  // 若打开失败
  // _PATH_UTMP 在mac不适用  mac为/var/run/utmpx文件
  if ((fp = fopen("/var/run/utmpx", "r")) == NULL)
  {
    perror("fopen()");
    return 1;
  }

  // 每次读入正确大小并写入myUtmp中
  while (fread(&tmp, utmpLen, 1, fp) == 1)
  {
    // 打印用户信息
    showInfo(&tmp);
  }

  // 关闭文件描述符
  fclose(fp);
  return 0;
}