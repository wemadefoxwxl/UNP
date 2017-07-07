#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <algorithm>
#include <vector>

typedef std::vector<struct epoll_event> EventList;

#define ERR_EXIT(m)\
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)

int main()
{
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	int idelfd = open("/dev/null",O_RDONLY|O_CLOEXEC);

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int on = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	if(listen(sockfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");

	std::vector<int> clients;
	int epollfd = epoll_create(EPOLL_CLOEXEC);

	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&event);

	EventList events(16);
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int connfd;

	int nready;
	while(1)
	{
		nready = epoll_wait(epollfd,&*events.begin(),events.size(),-1);
		if(-1 == nready)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("epoll_wait");
		}

		if(0 == nready)
			continue;

		if(static_cast<size_t>(nready) == events.size())
			events.resize(events.size()*2);

		for(int i = 0; i < nready;++i)
		{
			if(events[i].data.fd == sockfd)
			{
				peerlen = sizeof(peeraddr);
				connfd = accept4(sockfd,(struct sockaddr*)&peeraddr,&peerlen,SOCK_NONBLOCK|SOCK_CLOEXEC);

				if(-1 == connfd)
				{
					close(idelfd);
					idelfd = accept(sockfd,NULL,NULL);
					close(idelfd);
					idelfd = open("/dev/null/",O_RDONLY|O_CLOEXEC);
					continue;
				}

				std::cout << "client connect  : ";
				std::cout << " ip = " << inet_ntoa(peeraddr.sin_addr)
						  << " port = " << ntohs(peeraddr.sin_port);
				event.data.fd = connfd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&event);	
			}
			else if(events[i].events & EPOLLIN)
			{
				connfd = events[i].data.fd;
				if(connfd < 0)
					continue;

				char buf[1024] = {0};
				int ret = read(connfd,buf,1024);
				if(-1 == ret)
					ERR_EXIT("read");
				if(0 == ret)
				{
					std::cout << "client closed " << std::endl;
					close(connfd);
					event = events[i];
					epoll_ctl(epollfd,EPOLL_CTL_DEL,connfd,&event);
					continue;
				}
				std::cout << buf;
				write(connfd,buf,strlen(buf));
			}
		}

	}

	return 0;
}