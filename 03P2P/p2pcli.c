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

int main()
{
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("connect");

	pid_t pid;
	pid = fork();
	if (-1 == pid)
		ERR_EXIT("fork");

	// 子进程用来接收数据
	if(0 == pid)
	{
		char recvbuf[1024];
		while(1)
		{
			memset(recvbuf,0,sizeof(recvbuf));
			int ret = read(sock,recvbuf,sizeof(recvbuf));
			if(-1 == ret)
				break;
			else if(0 == ret)
			{
				printf("peer close \n");
				break;
			}
			fputs(recvbuf, stdout);
		}
		close(sock);
		// 发送信号杀死父进程
		kill(getppid(),SIGUSR1);
	}
	// 父进程发送数据
	else
	{
		// 安转信号
		signal(SIGUSR1,handler);
		char buf[1024];
		memset(buf,0,sizeof(buf));
		while((fgets(buf,sizeof(buf),stdin)) != NULL)
		{
			write(sock,buf,strlen(buf));
			memset(buf,0,sizeof(buf));
		}
		close(sock);
	}
	return 0;
}