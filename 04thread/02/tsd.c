#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

typedef struct tsd
{
	pthread_t tid;
	char* str;
}tsd_t;

pthread_key_t key_tsd;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

void destroy_routine(void* value)
{
	printf("destroy .....\n");
	free(value);
}

void once_routine()
{
	// 创建一个线程的特定数据，每个线程都具有的全局变量、
	// destroy_routine销毁时调用，如何销毁value
	pthread_key_create(&key_tsd,destroy_routine);
	printf("key init ... 0x%x\n",(int)pthread_self());
}

void* thread_routine(void* arg)
{
	// 只有第一个线程才会执行once_control,以后其他的线程不会执行下面的第一句程序
	// 产生一个key_tsd;
	pthread_once(&once_control,once_routine);
	tsd_t* value = (tsd_t*)malloc(sizeof(tsd_t));
	value->tid = pthread_self();
	value->str = (char*)arg;

	pthread_setspecific(key_tsd,value);
	printf("%s pthread_setspecific %p\n",(char*)arg,value);
	value = pthread_getspecific(key_tsd);
	printf("tid = 0x%x , str = %s\n",(int)value->tid,value->str );
	sleep(3);
	value = pthread_getspecific(key_tsd);
	printf("tid = 0x%x , str = %s\n",(int)value->tid,value->str );
	return NULL;
}

int main()
{
	pthread_t tid1;
	pthread_t tid2;

	pthread_create(&tid1,NULL,thread_routine,"thread 1");
	pthread_create(&tid2,NULL,thread_routine,"thread 2");

	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);

	printf("AAAAA\n");
	pthread_key_delete(key_tsd);
	printf("BBBBB\n");
	return 0;
}
