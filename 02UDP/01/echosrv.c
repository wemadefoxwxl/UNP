#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define ERR_EXIT(m)\
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)

void echo_srv(int sockfd)
{
	char recvbuf[1024];
	struct sockaddr_in peeraddr;
	socklen_t len;
	int n;
	while(1)
	{
		len = sizeof(peeraddr);
		memset(recvbuf,0,sizeof(recvbuf));
		n = recvfrom(sockfd,recvbuf,sizeof(recvbuf),0,(struct sockaddr*)&peeraddr,&len);
		if(-1 == n)
		{
			if(EINTR == errno)
				continue;
			ERR_EXIT("recvfrom");
		}

		if(0 == n)
		{
			fprintf(stderr, "client closed\n");
			break;
		}
		else if(n > 0)
		{
			fputs(recvbuf,stdout);
			sendto(sockfd,recvbuf,n,0,(struct sockaddr*)&peeraddr,len);
		}
	}
	close(sockfd);
}

int main()
{	
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("bind");

	echo_srv(sockfd);

	return 0;
}