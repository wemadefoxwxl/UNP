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

int main()
{
	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8080);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if((connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))) < 0)
		ERR_EXIT("connect");

	struct packet sendbuf;
	struct packet recvbuf;

	memset(&sendbuf,0,sizeof(sendbuf));
	memset(&recvbuf,0,sizeof(recvbuf));
	while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL)
	{	
		// printf("send : %s \n",sendbuf.buf );
		int len = strlen(sendbuf.buf);
		sendbuf.len = htonl(len);
		writen(sockfd,&sendbuf,4+len);

		int ret = readn(sockfd,&recvbuf.len,4);
		if(ret == -1)
			ERR_EXIT("readn");
		else if(ret < 4)
		{
			printf("server close \n");
			break;
		}
		len = ntohl(recvbuf.len);
		ret = readn(sockfd,recvbuf.buf,len);
		if(-1 == ret)
			ERR_EXIT("readn");
		else if(ret < len)
		{
			printf("server close\n");
			break;
		}
		fputs(recvbuf.buf,stdout);
		memset(&sendbuf,0,sizeof(sendbuf));
		memset(&recvbuf,0,sizeof(recvbuf));
	}
	close(sockfd);

	return 0;
}