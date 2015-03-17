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
	struct sockaddr_in servaddr;
	struct sockaddr_in clinaddr;
	char cmd[10] = {0};
	int timeout_num = 0;
	struct timeval timeout;
	fd_set rds;
	int ret = -1;
	int len = 0;
	int connfd = -1;
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&clinaddr, 0, sizeof(clinaddr));
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("create socket error. errno = %d\n", errno);
		exit(0);
	}
	printf("sockfd = %d\n", sockfd);
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	len = sizeof(struct sockaddr);
	
	if (bind(sockfd, (struct sockaddr *)(&servaddr), len) < 0)
	{
		printf("bind error. errno = %d\n", errno);
		exit(0);
	}
	
	listen(sockfd, 5);
	
	if ((connfd = accept(sockfd, (struct sockaddr *)(&clinaddr), &len)) < 0)
	{
		printf("bind error. errno = %d\n", errno);
		exit(0);
	}
	
	printf("start long tcp connect.\n");
	
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	

	while(1)
	{
		if (timeout_num > 3)
		{
			printf("long tcp connect timeout, will be close.\n");
			break;
		}
		memset(&cmd, 0, sizeof(cmd));
		FD_ZERO(&rds);
		FD_SET(sockfd, &rds);

		ret = select(sockfd + 1, &rds, NULL, NULL, &timeout);
		if (ret == 0)
		{
			timeout_num++;
			printf("long tcp timeout %d time\n", timeout_num);
		}
		else if (ret < 0)
		{
			printf("select error. errno = %d\n", errno);
			continue;
		}
		else
		{
			if(FD_ISSET(sockfd, &rds))
			{
				recv(connfd, cmd, sizeof(cmd), 0);
				printf("recv cmd, %s\n", cmd);
			}
		}

		if (strncmp(cmd, "alive", strlen("alive")) == 0)
		{
			timeout_num = 0;
			printf("recv the heartbeat packet success\n");
		}
		
		sleep(1);
	}
	
	close(sockfd);
	return 0;
}