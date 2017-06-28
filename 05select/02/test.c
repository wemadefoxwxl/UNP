#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

int main()
{
	printf("FD_SETSIZE = %d\n",FD_SETSIZE );
	return 0;
}