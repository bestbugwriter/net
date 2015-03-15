#include <sys/time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PORT 18008

int main(int argc, char *argv)
{
	int sockfd = -1;
	int num = 0;
	struct sockaddr_in clinaddr;
	struct sockaddr_in servaddr;
	char cmd[10] = {0};
	
	memset(&clinaddr, 0, sizeof(clinaddr));
	memset(&servaddr, 0, sizeof(servaddr));
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("create socket error. errno = %d\n", errno);
		exit(0);
	}
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("123.57.79.83");
	servaddr.sin_port = htons(PORT);
	
	if(-1 == connect(sockfd, (struct sockaddr *)(&servaddr), sizeof(struct sockaddr)))
	{
		printf("connect error. errno = %d\n", errno);
		exit(0);
	}
	
	strncpy(cmd, "alive\n", strlen("alive\n"));
	printf("cmd = %s\n", cmd);
	
	while(1)
	{
		if (send(sockfd, &cmd, strlen(cmd), 0) < 0)
		{
			printf("write error. errno = %d\n", errno);
			continue;
		}
		
		num++;
		printf("keep hreatbeat %d second\n", num);
		sleep(1);
	}
	
	close(sockfd);
	return 0;
}