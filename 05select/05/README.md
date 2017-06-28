select实现并发服务器，能达到的并发数受两方面的限制
1.一个进程所能打开的最大文件描述符的限制。这可以通过调整内核参数来调整。1）ulimit -n 获取一个进程能打开的文件描述符的个数
		ulimit -n num 设置一个进程所能打开的文件描述符的个数
	2）nofile_limit.c通过系统调用来设置和查看一个进程打开的文件描述符的个数。
2.select中fd_set集合的容量FD_SETSIZE，这需要重新编译内核。
 
coonntest.c echosrv.c对select并发数的测试。