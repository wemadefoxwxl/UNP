#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
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
	int listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(listenfd < 0)
	{
		ERR_EXIT("socket");
	}

	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0 )
		ERR_EXIT("bind");
	if(listen(listenfd,SOMAXCONN)<0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn ; 
	if((conn = accept(listenfd,(struct sockaddr*)&peeraddr,&peeraddr))<0)
		ERR_EXIT("accept");
	
	char recvbuf[1024];
	while(1)
	{
		memset(recvbuf,0,sizeof(recvbuf));
		int ret = read(conn,recvbuf,sizeof(recvbuf));
		fputs(recvbuf,stdout);
		write(conn,recvbuf,ret);
	}
	close(conn);
	close(listenfd);

	return 0;
}