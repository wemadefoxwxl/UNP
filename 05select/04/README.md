 用select封装的一些超时函数:
 1.read_timeout  select实现读超时检测函数，不含读操作
 2.write_timeout  select读超时检测函数，不含写操作
 3.accept_timeout  select实现带超时的accept
 4.connect_timeout  select 设置客户端connet连接超时