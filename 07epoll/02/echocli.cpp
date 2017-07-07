#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>

#define ERR_EXIT(m)\
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)

int main()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("connet");

	struct sockaddr_in localaddr;
	socklen_t len = sizeof(len);
	if(getsockname(sockfd,(struct sockaddr*)&localaddr,&len) < 0)
		ERR_EXIT("getsockname");

	std::cout << "getsockname :  ";
	std::cout << "ip = " << inet_ntoa(localaddr.sin_addr) << "  port = " << ntohs(localaddr.sin_port) << std::endl;

	char sendbuf[1024];
	char recvbuf[1024];

	while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
	{
		write(sockfd,sendbuf,strlen(sendbuf));
		read(sockfd,recvbuf,sizeof(recvbuf));

		fprintf(stderr, "%s", recvbuf);
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sockfd);

	return 0;
}