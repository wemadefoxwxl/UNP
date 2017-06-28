#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
	unsigned int x = 0x12345678;
	unsigned char* p = (unsigned char*)&x;
	//  测试主机字节序  小端字节序
	printf("%0x %0x %0x %0x \n",p[0],p[1],p[2],p[3] ); 

	unsigned int y = htonl(x);
	p = (unsigned char*)&y;
	// 测试网络字节序	 大端字节序
	printf("%0x %0x %0x %0x \n",p[0],p[1],p[2],p[3] );

	return 0;
}