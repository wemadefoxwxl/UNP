#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/semaphore.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

#define CONSUMERS_COUNT 1
#define PROCUDERS_COUNT 1
#define BUFSIZE 10

int g_buf[BUFSIZE];
int in = 0;
int out = 0;
int produce_id = 0;
int consume_id = 0;

sem_t g_sem_full;
sem_t g_sem_empty;
pthread_mutex_t g_mutex;


pthread_t tid[CONSUMERS_COUNT + PROCUDERS_COUNT];

void* consume(void *arg);
void* produce(void *arg);

int main()
{
	int i ;
	for(i = 0; i < BUFSIZE;++i)
			g_buf[i] = -1;

	sem_init(&g_sem_empty,0,0);
	sem_init(&g_sem_full,0,BUFSIZE);

	for(i = 0; i < CONSUMERS_COUNT;++i)
		pthread_create(&tid[i],NULL,consume,(void*)i);

	for(i = 0; i < PROCUDERS_COUNT;++i)
		pthread_create(&tid[i + CONSUMERS_COUNT],NULL,produce,(void*)i);

	for(i = 0; i < CONSUMERS_COUNT + PROCUDERS_COUNT;++i )
		pthread_join(tid[i],NULL);

	sem_destroy(&g_sem_empty);
	sem_destroy(&g_sem_full);
	pthread_mutex_destroy(&g_mutex);

	return 0;
}

void* consume(void *arg)
{
	int num = (int)arg;
	while(1)
	{
		printf("\n");
		printf("%d wait buffer not empty\n",num );
		sem_wait(&g_sem_empty);
		pthread_mutex_lock(&g_mutex);

		for(int i = 0; i < BUFSIZE;++i)
		{
			printf("%02d ", i);
			if(g_buf[i] == -1)
				printf("%s ", "NULL");
			else
				printf("%d ",g_buf[i] );

			if(i == out)
				printf("\t <--consume");
			printf("\n");
		}
		consume_id = g_buf[out];
		printf("%d begin consume product %d \n",num, consume_id);
		g_buf[out] = -1;
		out = (out+1) % BUFSIZE;
		printf("%d end   consume  product %d \n",num, consume_id);
		pthread_mutex_unlock(&g_mutex);
		sem_post(&g_sem_full);
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
		printf("\n");
		printf("%d wait buf not full \n",num );
		sem_wait(&g_sem_full);
		pthread_mutex_lock(&g_mutex);

		for(int i = 0 ; i < BUFSIZE; ++i)
		{
			printf("%02d ", i);
			if(-1 == g_buf[i])
				printf("%s  ", "NULL" );
			else
				printf("%d  ",g_buf[i]);
			if(in == i)
				printf("\t <-- produce");
			printf("\n");
		}
		printf("%d begin produce product %d\n",num,produce_id );
		g_buf[in] = produce_id;
		in = (in + 1) % BUFSIZE;
		printf("%d end   produce product %d\n",num,produce_id++);
		pthread_mutex_unlock(&g_mutex);
		sem_post(&g_sem_empty);
		printf("\n");
		sleep(5);
	}
	return NULL;
}








