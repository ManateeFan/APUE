#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    char curDir[100];

    if(getcwd(curDir, 100) == NULL)
    {
        perror("getDir");
        exit(1);
    }
    printf("curDir: %s\n", curDir);

    if(chdir("/Users/") == -1)
    {
        perror("chdir");
        exit(1);
    }
    
    char curDir2[100];
    if(getcwd(curDir2, 100) == NULL)
    {
        perror("getDir");
        exit(1);
    }
    printf("curDir: %s\n", curDir2);
    exit(0);


}
