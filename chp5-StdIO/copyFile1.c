#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        printf("need 2 file\n");
        exit(1);
    }
    FILE* f1 = NULL;
    FILE* f2 = NULL;

    if (!(f1 = fopen(argv[1], "r")))
    {
        perror("open1()");
        exit(1);
    }
    if (!(f2 = fopen(argv[2], "w")))
    {
        perror("open2()");
        exit(1);
    }

    int ch = 0;
    while ((ch = fgetc(f1)) != EOF)
    {
        fputc(ch, f2);
    }

    fclose(f1);
    fclose(f2);

    exit(0);
}