#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXSIZE 1024

int main()
{
  char buf[MAXSIZE + 1];
  ssize_t numRead;

  numRead = read(STDIN_FILENO, buf, MAXSIZE);
  if (numRead == -1)
  {
    perror("read()");
    exit(EXIT_FAILURE);
  }

  buf[MAXSIZE] = '\0';

  printf("%s", buf);
  exit(EXIT_SUCCESS);
}