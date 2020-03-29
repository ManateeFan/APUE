#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    if (lseek(STDIN_FILENO, 0, SEEK_CUR) == -1)
        printf("Can not seek\n");
    else
    {
        printf("Seek ok\n");
    }

    exit(0);

}