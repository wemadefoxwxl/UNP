#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
        }while(0) 

void echo_cli(int fd)
{
	char sendbuf[1024];
	char recvbuf[1024];

	while(1)
	{
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));

		if(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
		{
			write(fd,sendbuf,sizeof(sendbuf));
			read(fd,recvbuf,sizeof(recvbuf));

			fputs(recvbuf,stdout);
		}
	}
}

int main()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("connect");
	echo_cli(sockfd);

	return 0;
}