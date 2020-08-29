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
	sem_t* sem = sem_open(argv[1], 0);
	sem_post(sem);
	int val;
	sem_getvalue(sem, &val);
	printf("values = %d\n", val);
	exit(EXIT_SUCCESS);
}