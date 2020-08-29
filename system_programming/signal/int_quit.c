#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// signal handle
static void sig_handle(int signo)
{
    static int count = 0;
    if (signo == SIGINT)
    {
        count++;
        printf("Caught signal SIGINT (%d)\n", count);
        return;
    }

    printf("Caught signal SIGQUIT\n");
    exit(EXIT_SUCCESS);
}

int main()
{
    if (signal(SIGINT, sig_handle) == SIG_ERR)
    {
        perror("signal()");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, sig_handle) == SIG_ERR)
    {
        perror("signal()");
        exit(EXIT_FAILURE);
    }
    while (1)
        pause();

    exit(EXIT_SUCCESS);
}
