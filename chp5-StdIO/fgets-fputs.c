#include <stdio.h>
#include <stdlib.h>

int main()
{
  char buf[1024];

  while (fgets(buf, 1024, stdin))
  {
    if (fputs(buf, stdout) == EOF)
    {
      perror("output");
      exit(1);
    }
  }
  if (ferror(stdin))
  {
    perror("input");
    exit(1);
  }
  exit(0);
}