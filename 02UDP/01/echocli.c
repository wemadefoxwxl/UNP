#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)\
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)

void echo_cli(int sockfd)
{
	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = 0;
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};

	while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
	{
		sendto(sockfd,sendbuf,strlen(sendbuf),0,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
		ret = recvfrom(sockfd,recvbuf,sizeof(recvbuf),0,NULL,NULL);

		if(-1 == ret)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}
		fputs(recvbuf,stdout);
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sockfd);
	printf("client closed\n");
}

int main()
{
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0) 
		ERR_EXIT("socket");

	echo_cli(sockfd);


	return 0;
}