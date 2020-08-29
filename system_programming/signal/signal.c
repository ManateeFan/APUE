#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

void int_signal(int s)
{
	write(STDOUT_FILENO, "!", 1);
}
int main()
{
	signal(SIGINT, int_signal);

	int i = 0;
	for (; i < 10; i++)
	{
		write(STDOUT_FILENO, "*", 1);
		sleep(1);
	}
	exit(0);
}
