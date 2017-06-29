#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main()
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int state;
	// 获取线程的分离属性
	pthread_attr_getdetachstate(&attr,&state);
	if(state == PTHREAD_CREATE_JOINABLE)
		printf("detachstate : PTHREAD_CREATE_JOINABLE\n");
	else if(state == PTHREAD_CREATE_DETACHED)
		printf("detachstate : PTHREAD_CREATE_JOINABLE\n");

	size_t size;
	// 获取线程栈的大小
	pthread_attr_getstacksize(&attr,&size);
	printf("stack size : %zu\n",size );

	// 获取线程的栈溢出保护区范围
	pthread_attr_getguardsize(&attr,&size);
	printf("guardsize : %zu\n",size );


	int scpoe;
	// 获取线程的竞争范围
	pthread_attr_getscope(&attr,&scpoe);
	if(scpoe == PTHREAD_SCOPE_PROCESS)
		printf("scope : PTHREAD_SCOPE_PROCESS\n");
	else if(scpoe == PTHREAD_SCOPE_SYSTEM)
		printf("scpoe : PTHREAD_SCOPE_SYSTEM\n");

	int policy;
	// 设置调度策略
	pthread_attr_getschedpolicy(&attr,&policy);
	if(policy == SCHED_FIFO)
		printf("policy : SCHED_FIFO\n");
	else if(policy == SCHED_RR)
		printf("policy : SCHED_RR\n");
	else if(policy == SCHED_OTHER)
		printf("policy : SCHED_OTHER\n");

	int inheritsched;
	// 获取线程继承的调度范围
	pthread_attr_getinheritsched(&attr, &inheritsched);
	if (inheritsched == PTHREAD_INHERIT_SCHED)
		printf("inheritsched : PTHREAD_INHERIT_SCHED\n");
	else if (inheritsched == PTHREAD_EXPLICIT_SCHED)
        printf("inheritsched : PTHREAD_EXPLICIT_SCHED\n");

    struct sched_param param;
    // 获取线程调度参数
    pthread_attr_getschedparam(&attr,&param);
    printf("sched_priority : %d\n", param.sched_priority);

    pthread_attr_destroy(&attr);

    int level;
    // 获取线程的并发级别
    // 0：表示内核按照自己的方式进行映射
    level = pthread_getconcurrency();
    printf("level : %d \n", level);

	return 0;
}