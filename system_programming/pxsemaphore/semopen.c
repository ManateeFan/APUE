#include<semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
	int flags = O_CREAT | O_RDWR;
	int value = 1;
	int c;
	while((c = getopt(argc, argv, "ei:")) != -1)
	{
		switch(c)
		{
			case 'e':
			flags |= O_EXCL;
			break;
			case 'i':
			value = atoi(optarg);
			break;
		}
	}
	if(optind != argc - 1)
	{
		fprintf(stderr, "usage: %s [-e] [-i initalvalue] <name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	sem_t* sem = sem_open(argv[optind], flags, 0664, value);
	sem_close(sem);	
	return 0;
}