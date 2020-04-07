#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main()
{
    if (open("tmpfile", O_CREAT | O_RDWR | O_TRUNC, 0644) < 0)
    {
        sys_err("open()");
    }
    if (unlink("tmpfile") < 0)
    {
        sys_err("unlink()");
    }
    printf("file unlinked\n");
    sleep(2);
    printf("done!\n");
    exit(0);
}