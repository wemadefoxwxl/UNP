#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 3

pthread_mutex_t g_mutex;
pthread_cond_t  g_cond;
int nready = 0;

pthread_t tid[CONSUMERS_COUNT + PRODUCERS_COUNT];

void* consume(void* arg);
void* produce(void* arg);

int main()
{
	pthread_mutex_init(&g_mutex,NULL);
	pthread_cond_init(&g_cond,NULL);

	for(int i = 0; i < PRODUCERS_COUNT;++i)
		pthread_create(&tid[i],NULL,produce,(void*)i);

	for(int i =0 ; i < CONSUMERS_COUNT;++i)
		pthread_create(&tid[i],NULL,consume,(void*)i);

	for(int i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT;++i)
		pthread_join(tid[i],NULL);
	pthread_mutex_destroy(&g_mutex);
	pthread_cond_destroy(&g_cond);

	return 0;
}

void* consume(void* arg)
{
	int num = (int)arg;

	while(1)
	{
		pthread_mutex_lock(&g_mutex);
		while(nready == 0)
		{
			printf("%d begin wait a condtion ...\n\n", num);
			pthread_cond_wait(&g_cond,&g_mutex);
			// printf("%d end wait a condtion ...\n", num);
		}
		printf("%d begin consume product ... \n",num);
		--nready;
		printf("%d end consume product ....\n", num);
		pthread_mutex_unlock(&g_mutex);
		printf("\n");
		sleep(5);
	}
	return NULL;
}

void* produce(void* arg)
{
	int num = (int)arg;
	while(1)
	{
		pthread_mutex_lock(&g_mutex);
		printf("%d begin produce product\n",num);
		++nready;
		printf("%d end   produce product\n",num);
		if(nready > 0)
		{
			pthread_cond_signal(&g_cond);
			printf("signal ...\n");
		}
		pthread_mutex_unlock(&g_mutex);
		printf("\n");
		sleep(10);
	}
	return NULL;
}











