#include<semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	sem_unlink(argv[1]);
	return 0;
}