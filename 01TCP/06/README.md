解决多进程并发的服务器，客户端有大量退出连接时，出现僵尸进程的处理。
使用三种方法1. signal(SIGCHLD,SIG_IGN)
		  2.  signal(SIGCHLD,handler);
		  	void handler(int signal)
		  	{
		  		wait(NULL);
		  	}
			此方法不能处理大量的客户端断开。
		3. signal(SIGCHLD,handler);
			void handler(int signal)
			{
				while(waitpid(-1,NULL,WNOHUNG) < 0)
							;
			}
			可以处理大量的客户端同时断开。