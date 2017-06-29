#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        }while(0)

void echo_sev(int fd)
{
	char buf[1024];
	while(1)
	{
		memset(buf,0,sizeof(buf));
		int ret = read(fd,buf,sizeof(buf));
		if(0 == ret)
		{
			printf("client closed \n");
			break;
		}
		else if(-1 == ret)
			ERR_EXIT("read");
		fputs(buf,stdout);
		write(fd,buf,ret);
	}
}

void* thread_routinue(void* arg)
{
	pthread_detach(pthread_self());
	int conn = *((int*)arg);
	free(arg);
	echo_sev(conn);
	printf("exit thread conn is %d\n",conn);
	return NULL;
}

int main()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
		ERR_EXIT("sockfd");

	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int on = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("getsockopt");
	if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("bind");
	if(listen(sockfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	int conn;

	while(1)
	{
		if((conn = accept(sockfd,(struct sockaddr*)&peeraddr,&len)) < 0)
			ERR_EXIT("accept");
		printf("client accpet : ip = %s port = %d \n", inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));

		pthread_t tid;
		int ret;
		// 防止多线程的竟态
		int* p = (int*)malloc(sizeof(int));
		*p = conn;

		if( (ret = pthread_create(&tid,NULL,thread_routinue,p)) < 0)
		{
			fprintf(stderr, "pthread_create %s\n",strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}