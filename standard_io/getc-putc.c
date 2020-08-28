#include <stdio.h>
#include <stdlib.h>

int main()
{
  int ch;

  while ((ch = getc(stdin)) != EOF)
  {
    if (putc(ch, stdout) == EOF)
      perror("stdout");
    exit(1);
  }
  if (ferror(stdin))
  {
    perror("stdin");
    exit(1);
  }
  exit(0);
}
