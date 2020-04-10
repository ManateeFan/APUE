#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// signal handle
static void sig_usr(int);

// error
void sys_err(const char *s);

int main()
{
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
    {
        sys_err("Can not catch user1 signal");
    }
    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
    {
        sys_err("Can not catch user2 signal");
    }

    while (1)
        pause();

    exit(0);
}

static void sig_usr(int signo)
{
    if (signo == SIGUSR1)
    {
        printf("Received signal user1\n");
    }
    else if (signo == SIGUSR2)
    {
        printf("Received signal user2\n");
    }
    else
    {
        printf("Unknow signal");
    }
}

void sys_err(const char *s)
{
    perror(s);
    exit(1);
}
