#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


#define ERR_EXIT(m)\
	do\
	{\
		perror(m);\
		exit(EXIT_FAILURE);\
	}while(0)

// 信号处理函数
void handler(int sig)
{
	printf("recv a signal %d\n",sig );
	exit(0);
}

int main(void)
{
	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

	int conn;
	if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
		ERR_EXIT("accept");

	printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	pid_t pid = fork();
	if(-1 == pid)
		ERR_EXIT("fork");

	// 子进程发送数据
	if(0 == pid)
	{
		signal(SIGUSR1,handler);
		char buf[1024];
		memset(buf,0,sizeof(buf));
		while((fgets(buf,sizeof(buf),stdin)) != NULL)
		{
			write(conn,buf,strlen(buf));
			memset(buf,0,sizeof(buf));
		}
		printf("child close \n");
		exit(0);
	}
	// 父进程接收数据
	else
	{
		char recvbuf[1024];
		while (1)
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			int ret = read(conn, recvbuf, sizeof(recvbuf));
			if (ret == -1)
				break;
			else if (ret == 0)
			{
				printf("peer close\n");
				break;
			}
			
			fputs(recvbuf, stdout);
		}
		printf("parent close\n");
		kill(pid, SIGUSR1);
		exit(EXIT_SUCCESS);
	}

	return 0;
}