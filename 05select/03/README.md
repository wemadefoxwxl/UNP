shutdown 与close的区别。
利用shutdown(fd,SHUN_WR);实现半关闭，避免了SIGPIEP信号的产生。