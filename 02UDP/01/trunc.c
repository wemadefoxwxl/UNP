#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	sendto(sockfd,"ABCD",4,0,(struct sockaddr*)&servaddr,sizeof(servaddr));

	char buf[1];
	int n;
	for(int i = 0; i < 4;++i)
	{
		n = recvfrom(sockfd,buf,sizeof(buf),0,NULL,NULL);
		if(-1 == n)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}
		if(0 == n)
		{
			fprintf(stderr, "client closed\n" );
			break;
		}
		else
		{
			printf("n = %d  %c \n",n,buf[0] );
		}
	}

	close(sockfd);

	return 0;
}