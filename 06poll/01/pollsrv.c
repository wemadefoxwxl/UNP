#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

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

void hander(int sig)
{
	printf("recv sig is %d\n",sig);
	printf("%s\n",strerror(errno) );
}


int main()
{
	signal(SIGPIPE,hander);
	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int on = 0;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("setsockopt");
	if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
		ERR_EXIT("bind");
	if(listen(sockfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);

	int nready = 0;
	struct pollfd client[8192];
	for(int i = 0; i < 8192;++i)
		client[i].fd = -1;
	client[0].fd = sockfd;
	client[0].events = POLLIN;
	int maxi = 0;
	int conn;
	int count = 0;

	while(1)
	{
		nready = poll(client,maxi+1,-1);
		if(-1 == nready)
		{
			if(EINTR == errno)
				continue;
			ERR_EXIT("select");
		}

		if(0 == nready)
			continue;
		// 监听套接字
		if(client[0].revents & POLLIN)
		{
			len = sizeof(peeraddr);
			conn = accept(sockfd,(struct sockaddr*)&peeraddr,&len);
			if(-1 == conn)
				ERR_EXIT("accept");
			int i = 0;
			for(; i < 8192;++i)
			{
				if(client[i].fd < 0)
				{
					client[i].fd = conn;
					if(i > maxi)
						maxi = i;
					break;
				}
			}
			if(i == 8192)
			{
				fprintf(stderr, "too many client\n");
				exit(EXIT_FAILURE);
			}
			printf("client : ip = %s  port = %d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port) );
			printf("count = %d \n",++count );

			client[i].events = POLLIN;
			if(--nready <= 0)
				continue;
		}

		// 客户端程序，已连接套接字
		for(int i = 0;i <= maxi;++i)
		{
			conn = client[i].fd;
			if(-1 == conn)
				continue;

			if(client[i].revents & POLLIN)
			{
				char buf[1024];
				memset(buf,0,sizeof(buf));
				int ret = readline(conn,buf,1024);
				if(-1 == ret)
					ERR_EXIT("readline");
				if(0 == ret)
				{	
					printf("client closed\n");
				    client[i].fd = -1;
				    close(conn);
				}
				else
				{
					fputs(buf,stdout);
					writen(conn,buf,strlen(buf));
				}
				if(--nready <=0)
					break;
			}
		}
	}
	


	return 0;
}
























