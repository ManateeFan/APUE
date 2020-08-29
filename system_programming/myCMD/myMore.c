#include <stdio.h>
#include <stdlib.h>

#define PAGELEN 24
#define BUFLEN 1024

int selectMore(FILE *fp)
{
    int c = fgetc(fp);
    if (c == 'q')
        return 0;
    if (c == '\n')
        return 1;
    if (c == ' ')
        return PAGELEN;
    else
        return 0;
}

int mainMore(FILE *fp)
{
    // 缓冲区
    char buf[BUFLEN];

    // 记录已打印行数
    int row = 0;

    // 接受用户的选项q, ' ', '\n'
    int reply = 0;

    // 从文件流获取一行
    while (fgets(buf, BUFLEN, fp) != NULL)
    {
        // 首先判断是否已经打印24行
        if (row == PAGELEN)
        {
            // 接受打印多少行
            reply = selectMore(stdin);

            // 不打印 退出
            if (reply == 0)
            {
                exit(0);
            }
            // 打印一行 或 一页
            else
            {
                row -= reply;
            }
        }
        else
        {
            // 打印一行
            fputs(buf, stdout);
            row++;
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *fp = NULL;

    if (argc == 1)
    {
        mainMore(stdin);
    }
    // 至少提供打印的文件1个
    else
    {
        while ((fp = fopen(argv[--argc], "r")))
        {
            mainMore(fp);
            fclose(fp);
        }
    }
    exit(0);
}
