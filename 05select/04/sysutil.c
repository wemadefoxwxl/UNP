#include "sysutil.h"


// read_timeout  select实现读超时检测函数，不含读操作
// fd 文件描述符
// wait_seconds 等待的秒数，如果是0表示不检测超时
// 成功未超时表示成功，
// 失败返回-1，
// 如果超时返回-1并且errno=ETIMEDOUT
int read_timeout(int fd,unsigned int wait_seconds)
{
	int ret = 0;
	if(wait_seconds > 0)
	{
		fd_set rset;
		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		FD_ZERO(&rset);
		FD_SET(fd,&rset);

		do
		{
			ret = select(fd+1,&rset,NULL,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);

		// 超时
		if(0 == ret)
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if(1 == ret)
		{
			ret = 0;
		}
	}

	return ret;
}

// write_timeout  select读超时检测函数，不含写操作
// fd ：文件描述符
// wait_seconds : 等待超时秒数，如果为0表示不检测超时
// 成功未超时返回0
// 失败返回-1
// 超时返回-1，并且errno=ETIMEDOUT
int write_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if(wait_seconds > 0)
	{
		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(fd,&wset);

		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do
		{
			ret = select(fd+1,NULL,&wset,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);

		if(0 == ret)
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if(0 == ret)
			ret = 0;
	}
	return ret;
}


// accept_timeout  select实现带超时的accept
// fd套接字
// addr:输出参数,返回对方的地址
// wait_seconds:等待超时的描述，如果为0表示正常模式
// 成功未超时返回已经连接的套接字，
// 超时返回-1并且errno=ETIMEDOUT
// 错误返回-1
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret = 0;
	socklen_t len = sizeof(struct sockaddr_in);

	if(wait_seconds > 0)
	{
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(fd,&rset);

		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do
		{
			ret = select(fd+1,&rset,NULL,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);

		if(0 == ret)
		{
			errno = ETIMEDOUT;
			return -1;
		}
		if(-1 == ret)
			return -1;
	}

	if(NULL != addr)
		ret = accept(fd,(struct sockaddr*)&addr,&len);
	else 
		ret = accept(fd,NULL,NULL);
	if(-1 == ret)
		ERR_EXIT("accpet");
	return ret;
}

// 设置I/O为非堵塞I/O
// fd：文件描述符
void activate_nonblock(int fd)
{
	int flag = fcntl(fd,F_GETFL);
	if(-1 == flag)
		ERR_EXIT("fcntl");
	flag |= O_NONBLOCK;
	int ret = fcntl(fd,F_SETFL,flag);
	if(-1 == ret)
		ERR_EXIT("fcntl");
}

// 设置I/O为堵塞I/O
// fd:文件描述符
void deactivate_nonblock(int fd)
{
	int flag = fcntl(fd,F_GETFL);
	if(-1 == flag)
		ERR_EXIT("fcntl");
	flag &=~O_NONBLOCK;
	int ret = fcntl(fd,F_SETFL,flag);
	if(-1 == ret)
		ERR_EXIT("fcntl");
}


// connect_timeout  select 设置connet连接超时
// fd:套接字
// addr:要连接的对方的地址,
// wait_seconds:等待的超时秒数,如果为0表示正常模式
// 成功为超时返回0
// 失败返回-1
// 超时返回-1，并且errno=ETIMEOUT
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	socklen_t len = sizeof(struct sockaddr_in);

	if(wait_seconds > 0)
		activate_nonblock(fd);

	int ret = connect(fd, (struct sockaddr*)addr,len);
	if(ret < 0 && errno == EINPROGRESS)
	{
			fd_set wset;
			FD_ZERO(&wset);
			FD_SET(fd,&wset);

			struct timeval timeout;
			timeout.tv_sec = wait_seconds;
			timeout.tv_usec = 0;

			do
			{
				// 一旦连接建立，套接字就可写
				ret = select(fd+1,NULL,&wset,NULL,&timeout);
			}while(ret < 0 && errno == EINTR);

			if(0 == ret)
			{
				ret = -1;
				errno = ETIMEDOUT;
			}
			else if(-1 == ret)
				return -1;
			else if(1 == ret)
			{
				// ret返回为1，两种情况
				// 1.连接建立成功，2.套接字产生错误
				// 此时错误信息不回保存在errno变量中，因此，需要调用soktgetopt来获取
				int err;
				socklen_t len = sizeof(err);
				int sockoptret = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&len);
				if(-1 == sockoptret)
					return -1;
				if(0 == err)	//无错误
					ret = 0;
				else	//产生错误
				{
					errno = err;
					ret = -1;
				}
			}
	}		
	if(wait_seconds > 0)
		deactivate_nonblock(fd);

	return ret;
}

// read 读取固定字节数
// fd : 文件描述符
// buf: 接收缓冲区
// count: 要发送的字节数
// 成功返回count，失败返回-1，读到EOF返回<count
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


//writen 发送固定字节数
// fd:文件描述符
// buf:发送缓存区
// count:发送的字节数
// 成功返回count，失败返回-1
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


// recv_peek:仅仅查看套接字缓冲区数据，但并不移除数据
// fd:套接字
// buf:接收缓冲区
// 成功返回>=0,失败返回-1
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

// readline 读取一行数据
// fd:套接字
// buf：接收缓冲区
// maxline：每行最大长度
// 成功返回>=0,失败返回-1
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
			// 如果有\n则读取
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
		// 又一次readn是为了清除接收缓冲区的内容
		ret = readn(fd,bufp,nread);
		if(ret != nread)
			exit(EXIT_FAILURE);
		bufp += nread;
	}
	return -1;
}

