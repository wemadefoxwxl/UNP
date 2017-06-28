#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

struct packet
{
	int len;
	char buf[1024];
};

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

void do_server(int conn)
{
	struct packet recvbuf;

	while(1)
	{
		memset(&recvbuf,0,sizeof(recvbuf));
		int ret = readn(conn,&recvbuf.len,4);
		if(-1 == ret)
			ERR_EXIT("readn");
		else if(ret < 4)
		{
			printf("client closed\n");
			break;
		}
		int len = ntohl(recvbuf.len);
		// printf("get len %d\n",len );
		ret = readn(conn,recvbuf.buf,len);
		if(ret == -1)
			ERR_EXIT("readn");
		else if(ret < len)
		{
			printf("client closed\n");
			break;
		}
		fputs(recvbuf.buf,stdout);
		writen(conn,&recvbuf,4+len);
	}
}


int main()
{
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
	int conn;
	pid_t pid;

	while(1)
	{
		if((conn = accept(sockfd,(struct sockaddr*)&peeraddr,&len)) < 0)
			ERR_EXIT("accept");

		printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

		pid = fork();
		if(-1 == pid)
			ERR_EXIT("fork");
		if(0 == pid)
		{
			// printf("child proess \n");
			close(sockfd);
			do_server(conn);
			exit(0);
		}
		else 
			close(conn);
	}

	return 0;
}
























