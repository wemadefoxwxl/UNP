#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>

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
		int i;
		for(i = 0; i < nread;++i)
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

//select监视stdin和sockfd
void echo_cli(int fd)
{
	fd_set rset;
	FD_ZERO(&rset);

	int nready;
	int maxfd;
	int fd_stdin = fileno(stdin);
	if(fd_stdin > fd)
		maxfd = fd_stdin;
	else
		maxfd = fd;

	char recvbuf[1024];
	char sendbuf[1024];
	int stdineof = 0;

	while(1)
	{
		memset(recvbuf,0,sizeof(recvbuf));
		memset(sendbuf,0,sizeof(sendbuf));

		// select 监视stdin和通信套接字的可读事件
		// 每次select之前，必须都加入关注
		if(0 == stdineof)
			FD_SET(fd_stdin,&rset);
		FD_SET(fd,&rset);

		nready = select(maxfd+1,&rset,NULL,NULL,NULL);
		if(-1 == nready)
			ERR_EXIT("select");
		if(0 == nready)
			continue;
		// 套接字有读事件
		if(FD_ISSET(fd,&rset))
		{
			int ret = readline(fd,recvbuf,sizeof(recvbuf));
			if(-1 == ret)
				ERR_EXIT("readline");
			else if(0 == ret)
			{
				printf("server closed\n");
				break;
			}

			fputs(recvbuf,stdout);
			memset(recvbuf,0,sizeof(recvbuf));
		}

		// 如果stdin有可读事件
		if(FD_ISSET(fd_stdin,&rset))
		{
			if(fgets(sendbuf,sizeof(sendbuf),stdin) == NULL)
			{
				// close(fd);
				stdineof = 1;
				shutdown(fd,SHUT_WR);		
			}
			writen(fd,sendbuf,strlen(sendbuf));
			memset(sendbuf,0,sizeof(sendbuf));
		}
	}
}



int main()
{
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
	socklen_t addrlen = sizeof(localaddr);
	if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) < 0)
		ERR_EXIT("getsockname");

	printf("loacl client :  ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

	echo_cli(sockfd);

	return 0;
}









