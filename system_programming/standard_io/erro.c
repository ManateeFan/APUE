#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    FILE* fp = NULL;
    fp = fopen("tmp", "w");
    if (!fp)
    {
        fprintf(stderr, "fopen(): failed! errno = %d\n", errno);
        perror("fopen()");
        exit(1);
    }

    puts("OK!");
    fclose(fp);
    exit(0);
}
