#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <string.h>
#include <net/if_arp.h>

int get_ipaddr(int sockfd, struct ifreq *ifr, struct sockaddr_in *sin)
{
	int ret = -1;
	if (sockfd < 0)
	{
		printf("sockfd < 0, error\n");
		return -1;
	}
	
	ret = ioctl(sockfd, SIOCGIFADDR, ifr);
	if (ret == -1)
	{
		printf("ioctl error, errno =%d\n", errno);
		return -1;
	}
	sin = (struct sockaddr_in *)&ifr->ifr_addr;
	printf("ip addr = %s\n", inet_ntoa(sin->sin_addr));
	
	return 0;
}

int init_socket(void)
{
	int fd = -1;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (fd < 0)
	{
		printf("create socket error\n");
		return -1;
	}
	return fd;
}

int main(int argc, char **argv)
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int sockfd = -1;
	
	memset(&ifr, 0, sizeof(ifr));
	memset(&sin, 0, sizeof(sin));
	
	memcpy(ifr.ifr_name, "eth0", sizeof("eth0"));
	sockfd = init_socket();
	printf("sockfd = %d\n", sockfd);
	
	get_ipaddr(sockfd, &ifr, &sin);
	
	return 0;
}