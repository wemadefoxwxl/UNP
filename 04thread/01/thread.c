#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define ERR_EXIT(m)\
	do\
	{\
		perror(m);\
		exit(EXIT_FAILURE);\
	}while(0)\

void* thread_routinue(void* arg)
{
	for(int i = 0;i < 20;++i)
	{
		printf("B");
		fflush(stdout);
		usleep(20);
		if(5 == i)
			pthread_exit("555");
	}
	sleep(3);
	return "ABC";
}


int main()
{
	pthread_t tid;
	int ret;
	if((ret = pthread_create(&tid,NULL,thread_routinue,NULL)) < 0)
	{
		fprintf(stderr, "pthread_create:%s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	for(int i = 0;i < 20;++i)
	{
		printf("A");
		fflush(stdout);
		usleep(20);
	}

	void* value;
	if((ret = pthread_join(tid,&value)) < 0)
	{
		fprintf(stderr, "pthread_join %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}
	printf("\n");
	printf("return msg = %s\n",(char*) value);

	return 0;
}