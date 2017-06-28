#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>

typedef std::vector<struct epoll_event> EventList;

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

void handler(int sig)
{
	printf("recv sig is %d\n",sig);
	printf("%s\n",strerror(errno) );
}

void activate_nonblock(int fd)
{
	int ret;
	int flags = fcntl(fd,F_GETFL);
	if(-1 == flags)
		ERR_EXIT("fcntl");
	flags |= O_NONBLOCK;
	ret = fcntl(fd,F_SETFL,flags);
	if(-1 == ret)
		ERR_EXIT("fcntl");

}


int main()
{
	signal(SIGPIPE,handler);
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

	// std::vector<int> clients;
	int epollfd;
	epollfd = epoll_create1(EPOLL_CLOEXEC);

	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&event);

	EventList events(16);
	int conn;

	int nready;
	int count = 0;
	while(1)
	{
		nready = epoll_wait(epollfd,&*events.begin(),static_cast<int>(events.size()),-1);
		if(-1 == nready)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("epoll_wait");
		}		

		if(0 == nready)
			continue;

		// 活跃事件较多
		if((size_t)nready == events.size())
			events.resize(events.size() * 2);

		for(int i = 0;i < nready;++i)
		{
			// 已连接套接字  客户端连接
			if(events[i].data.fd == sockfd)
			{
				conn = accept(sockfd,(struct sockaddr*)&peeraddr,&len);
				if(-1 == conn)
					ERR_EXIT("accept");

				printf("client : ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
				printf("count = %d\n",++count );
				// clients.push_back(conn);
				activate_nonblock(conn);
				event.data.fd = conn;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,conn,&event);
			}
			// 已连接套接字  与客户端进行通信
			else if(events[i].events & EPOLLIN)
			{
				conn = events[i].data.fd;
				if (conn < 0)
					continue;

				char recvbuf[1024] = {0};
				int ret = readline(conn, recvbuf, 1024);
				if (ret == -1)
				{
					if(errno == EINTR)
						continue;
					ERR_EXIT("readline");
				}
				if (ret == 0)
				{
					printf("client closed\n");
					close(conn);

					event = events[i];
					epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &event);
					// clients.erase(std::remove(clients.begin(), clients.end(), conn), clients.end());
				}

				fputs(recvbuf, stdout);
				writen(conn, recvbuf, strlen(recvbuf));
			}
		}
	}

	return 0;
}
























