#include <stdio.h>
#include <arpa/inet.h>

int main()
{
	unsigned long addr = inet_addr("192.168.1.1");
	// ip -> 32
	printf("addr = %u\n",ntohl(addr) );

	struct in_addr ipaddr;
	ipaddr.s_addr = addr;
	// 32 -> ip
	printf("%s\n",inet_ntoa(ipaddr) );
	return 0;
}