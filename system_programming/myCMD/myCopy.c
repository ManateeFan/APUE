#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int inputFd, outputFd, openFlags; // 文件描述符、文件打开状态
  mode_t filePerm; // 文件权限值
  ssize_t numRead; // 读写的字节数
  char buf[BUFSIZ]; // 缓冲区

  if (argc != 3 || strcmp(argv[1], "--help") == 0)
  {
    printf("%s old-file new-file\n", argv[0]);
    exit(EXIT_SUCCESS);
  }

  inputFd = open(argv[1], O_RDONLY);
  if (inputFd == -1)
  {
    fprintf(stderr, "%s open() %s", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  openFlags = O_CREAT | O_WRONLY | O_TRUNC;

  // rw-rw-rw-
  filePerm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  outputFd = open(argv[2], openFlags, filePerm);
  if (outputFd == -1)
  {
    fprintf(stderr, "%s open() %s", argv[2], strerror(errno));
    exit(EXIT_FAILURE);
  }

  while ((numRead = read(inputFd, buf, BUFSIZ)) > 0)
  {
    if (write(outputFd, buf, BUFSIZ) != numRead)
    {
      fprintf(stderr, "could not write whole buffer");
    }
  }
  if (numRead == -1)
  {
    perror("read()");
    exit(EXIT_FAILURE);
  }

  if (close(inputFd) == -1)
  {
    perror("close1()");
    exit(EXIT_FAILURE);
  }
  if (close(outputFd) == -1)
  {
    perror("close2()");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}