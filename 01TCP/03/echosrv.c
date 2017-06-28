#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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
	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//  设置地址复用
	int on = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)
		ERR_EXIT("setsockopt");

	if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("bind");
	if(listen(sockfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	int conn ;
	if((conn = accept(sockfd,(struct sockaddr*)&peeraddr,&len))<0)
		ERR_EXIT("accept");
	printf("client ip = %s  port = %d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));

	char buf[1024];
	while(1)
	{
		memset(buf,0,sizeof(buf));
		int ret = read(conn,buf,sizeof(buf));
		fputs(buf,stdout);
		write(conn,buf,ret);
	}

	return 0;
}