#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

#include <iostream>
#include <vector>

#define ERR_EXIT(m)\
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)

typedef std::vector<struct pollfd> PollFdlist;

int main()
{
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	// 可以优雅的解决EMFILE
	int idlefd = open("/dev/null",O_RDONLY|O_CLOEXEC);

	// socket的新用法
	int sockfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,0);
	if(sockfd < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
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

	struct pollfd pfd;
	pfd.fd = sockfd;
	pfd.events = POLLIN;

	PollFdlist pollfds;
	//pollfds[0] 为监听套接字
	pollfds.push_back(pfd);

	int nready = 0;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int connfd;

	while(1)
	{
		nready = poll(&*pollfds.begin(),pollfds.size(),-1);
		if(-1 == nready)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("poll");
		}
		// nothing happended
		if(0 == nready)
			continue;

		if(pollfds[0].revents & POLLIN)
		{
			peerlen = sizeof(peeraddr);
			// accept4的新用法
			connfd = accept4(sockfd,(struct sockaddr*)&peeraddr,
							&peerlen,SOCK_NONBLOCK|SOCK_CLOEXEC);

			if(-1 == connfd)
			{
				if(EMFILE == errno)
				{
					close(idlefd);
					idlefd = accept(sockfd,NULL,NULL);
					close(idlefd);
					idlefd = open("/dev/null",O_RDONLY|O_CLOEXEC);
					continue;
				}
				else
					ERR_EXIT("accept4");
			}

			pfd.fd = connfd;
			pfd.events = POLLIN;
			// revents一定设为0
			pfd.revents = 0;
			pollfds.push_back(pfd);
			--nready;

			// 连接成功 
			std::cout << " ip = " << inet_ntoa(peeraddr.sin_addr) 
					  << " port " << ntohs(peeraddr.sin_port)  << std::endl;

			if(0 == nready)
				continue;
		}

		for(PollFdlist::iterator it = pollfds.begin() + 1;
						it != pollfds.end() && nready > 0;++it)
		{
			if(it->revents & POLLIN)
			{
				--nready;
				connfd = it->fd;
				char buf[1024] = {0};
				int ret = read(connfd,buf,1024);
				if(-1 == ret)
					ERR_EXIT("read");
				if(0 == ret)
				{
					std::cout << "client closed " << std::endl;
					it = pollfds.erase(it);
					--it;
					close(connfd);
					continue;
				}

				std::cout << buf ;
				write(connfd,buf,strlen(buf));
			}
		}
	}

	return 0;
}