#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)


int main(void)
{
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		ERR_EXIT("getrlimit");

	printf("%d\n", (int)rl.rlim_max);

	rl.rlim_cur = 2048;
	rl.rlim_max = 2048;
	if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
		ERR_EXIT("setrlimit");

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		ERR_EXIT("getrlimit");

	printf("%d\n", (int)rl.rlim_max);
	return 0;
}
