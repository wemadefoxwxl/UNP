#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)\
	do\
	{\
		perror(m);\
		exit(EXIT_FAILURE);\
	}while(0)


int main()
{
	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("connect");

	char sendbuf[1024];
	char recvbuf[1024];
	while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
	{
		write(sockfd,sendbuf,strlen(sendbuf));
		read(sockfd,recvbuf,sizeof(recvbuf));

		fputs(recvbuf,stdout);
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sockfd);
	return 0;
}