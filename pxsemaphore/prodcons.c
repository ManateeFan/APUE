#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define NBUFF 10
#define SEM_MUTEX "mutex"
#define SEM_NEMPTY "nempty"
#define SEM_NSTORED "nstored"

// read-only by producer and consumer
int nitems;
// 缓冲区和信号量
struct {
	int buff[NBUFF];
	sem_t *mutex, *nempty, *nstored;
} shared;

void* produce(void*);
void* consume(void*);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <#nitems>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	nitems = atoi(argv[1]);

	// create semaphores
	shared.mutex = sem_open(px_ipc_name(SEM_MUTEX), O_CREAT | O_EXCL, 0664, 1);
	shared.nempty = sem_open(px_ipc_name(SEM_NEMPTY), O_CREAT | O_EXCL, 0664, NBUFF);
	shared.nstored = sem_open(px_ipc_name(SEM_NSTORED), O_CREAT | O_EXCL, 0666, 0);

	// create one producer and one consumer thread
	pthread_t tid_produce, tid_consume;
	pthread_create(&tid_produce, NULL, produce, NULL);
	pthread_create(&tid_consume, NULL, consume, NULL);

	// wait for two threads
	pthread_join(tid_produce, NULL);
	pthread_join(tid_consume, NULL);

	// remove the semaphores
	sem_unlink(px_ipc_name(SEM_MUTEX));
	sem_unlink(px_ipc_name(SEM_NEMPTY));
	sem_unlink(px_ipc_name(SEM_NSTORED));

	exit(EXIT_SUCCESS);
}

void* produce(void* arg)
{
	int i;
	for(i = 0; i < nitems; ++i)
	{
		sem_wait(shared.nempty);
		sem_wait(shared.mutex);
		shared.buff[i % NBUFF] = i;
		sem_post(shared.mutex);
		sem_post(shared.nstored);
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