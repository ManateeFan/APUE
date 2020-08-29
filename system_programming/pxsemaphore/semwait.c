#include<semaphore.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	sem_t* sem = sem_open(argv[1], 0);
	int val;
	sem_getvalue(sem, &val);
	sem_wait(sem);
	printf("pid %ld has semaphore, values = %d\n", (long)getpid(), val);
	pause();
	return 0;
}