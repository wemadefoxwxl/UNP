#include "sysutil.h"

int main()
{
	int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = connect_timeout(sockfd,&serveraddr,sizeof(serveraddr));
	if(ret == -1 && errno == ETIMEDOUT)
	{
		printf("timeout : \n");
		return 1;
	}
	else if(-1 == ret)
		ERR_EXIT("connect_timeout");
	else
	{
		printf("server conneted\n");
		// sleep(3);
	}

	struct sockaddr_in localaddr;
	socklen_t len = sizeof(localaddr);
	if(getsockname(sockfd,(struct sockaddr*)&localaddr,&len) < 0)
		ERR_EXIT("getsockname");

	printf("local client : ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

	return 0;
}