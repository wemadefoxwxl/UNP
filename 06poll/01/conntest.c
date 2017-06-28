#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)


int main()
{
	int count = 0;
	while(1)
	{
		int sockfd;
		if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		{
			// sleep(5)能够测出实际的并发数
			sleep(5);
			ERR_EXIT("socket");
		}

		struct sockaddr_in serveraddr;
		memset(&serveraddr,0,sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(8080);
		serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

		if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
			ERR_EXIT("connect");

		struct sockaddr_in localaddr;
		socklen_t len = sizeof(localaddr);
		if(getsockname(sockfd,(struct sockaddr*)&localaddr,&len) < 0)
			ERR_EXIT("getsockname");

		printf("client : ip = %s port = %d\n",inet_ntoa(localaddr.sin_addr),ntohs(localaddr.sin_port) );
		printf("client count = %d \n",++count);
	}
	return 0;
}