#include<stdio.h>
#include<arpa/inet.h>

int main()
{
    unsigned int val = 0x12345678;
    const char* p = (char*)&val;

    printf("%0x %0x %0x %0x\n", p[0], p[1], p[2], p[3]);
    return 0;
}
