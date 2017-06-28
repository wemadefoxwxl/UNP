#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define ERR_EXIT(m)\
	do\
	{\
		perror(m);\
		exit(EXIT_FAILURE);\
	}while(0)

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nread = read(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nread == 0)
			return count - nleft;

		bufp += nread;
		nleft -= nread;
	}

	return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nwritten = write(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nwritten == 0)
			continue;

		bufp += nwritten;
		nleft -= nwritten;
	}

	return count;
}

ssize_t recv_peek(int fd,void* buf,size_t len)
{
	while(1)
	{
		int ret = recv(fd,buf,len,MSG_PEEK);
		if(ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

// 读取一行
ssize_t readline(int fd,void* buf,size_t maxline)
{
	int ret;
	int nread;
	char* bufp = (char*)buf;
	int nleft = maxline;

	while(1)
	{
		ret = recv_peek(fd,bufp,nleft);
		if(ret < 0)
			return ret;
		else if(ret == 0)
			return ret;

		nread = ret;
		for(int i = 0; i < nread;++i)
		{
			if(bufp[i] == '\n')
			{
				ret = readn(fd,bufp,i+1);
				if(ret != i + 1)
					exit(EXIT_FAILURE);
				return ret;
			}
		}

		if(nread > nleft)
			exit(EXIT_FAILURE);

		nleft -= nread;
		ret = readn(fd,bufp,nread);
		if(ret != nread)
			exit(EXIT_FAILURE);
		bufp += nread;
	}
	return -1;
}

void echo_cli(int fd)
{
	char recvbuf[1024] = {0};
	char sendbuf[1024] = {0};

	while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
	{
		// writen(fd,sendbuf,sizeof(sendbuf));

		// 分两次发送信息，测试SIGPIPE产生
		write(fd,sendbuf,1);
		write(fd,sendbuf+1,strlen(sendbuf)-1);

		int ret = readline(fd,recvbuf,sizeof(recvbuf));

		if(-1 == ret)
			ERR_EXIT("readline");
		else if(0 == ret)
		{
			printf("server closed\n");
			break;
		}

		fputs(recvbuf,stdout);
		memset(&recvbuf,0,sizeof(recvbuf));
		memset(&sendbuf,0,sizeof(sendbuf));
	}
	close(fd);
}

void handler(int sig)
{
	printf("recv a sig %d\n",sig );
}

int main()
{
	// 安装信号
	// signal(SIGPIPE,handler);
	// SIGPIPE信号的处理方法，直接忽略即可
	signal(SIGPIPE,SIG_IGN);
	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("connect");

	struct sockaddr_in localaddr;
	socklen_t len = sizeof(localaddr);
	if(getsockname(sockfd,(struct sockaddr*)&localaddr,&len) < 0)
		ERR_EXIT("getsockname");
	printf("local :  ip = %s port = %d  \n", inet_ntoa(localaddr.sin_addr),ntohs(localaddr.sin_port));

	echo_cli(sockfd);

	return 0;
}