#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define NBUFF 10
#define MAXNTHREADS 100


// read-only by producer and consumer
int nitems, nproducers;
// 缓冲区和信号量
struct {
	int buff[NBUFF];
	int nput;	// next put position index nput % NBUFF
	int nputval;// net put value
	sem_t mutex, nempty, nstored;
} shared;

void* produce(void*);
void* consume(void*);

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s <#nitems> <#producers>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	nitems = atoi(argv[1]);
	nproducers = min(atoi(argv[2]), MAXNTHREADS);
	// create semaphores
	sem_open(shared.mutex, 0, 1);
	sem_open(shared.nempty, 0, NBUFF);
	sem_open(shared.nstored, 0, 0);

	// create one producer and one consumer thread
	pthread_t tid_produce[MAXNTHREADS], tid_consume;
	int count[MAXNTHREADS];	// every producer produce nitems
	int i;
	for(i = 0; i < nproducers; ++i)
	{
		count[i] = 0;
		pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}

	pthread_create(&tid_consume, NULL, consume, NULL);

	// wait for two threads
	for(i = 0; i < nproducers; ++i)
	{
		pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);
	}
	pthread_join(tid_consume, NULL);

	// remove the semaphores
	sem_destroy(&shared.mutex);
	sem_destroy(&shared.nempty);
	sem_destroy(&shared.nstored);

	exit(EXIT_SUCCESS);
}

void* produce(void* arg)
{
	int i;
	for(;;)
	{
		sem_wait(&shared.nempty);
		sem_wait(&shared.mutex);
		if(shared.nput >= nitems)
		{
			sem_post(&shared.nempty);
			sem_post(&shared.mutex);
			return NULL;
		}

		shared.buff[shared.nput % NBUFF] = shared.nputval;
		shared.nputval++;
		shared.nput++;
		sem_post(shared.mutex);
		sem_post(shared.nstored);
		*((int*)arg) += 1;
	}
	return NULL;
}

void* consume(void* arg)
{
	int i;
	for(i = 0; i < nitems; ++i)
	{
		sem_wait(shared.nstored);
		sem_wait(shared.mutex);
		if(shared.buff[i % NBUFF] != i)
			printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
		sem_post(shared.mutex);
		sem_post(shared.nempty);
	}
	reutrn NULL;
}