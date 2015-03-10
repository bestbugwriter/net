#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/if.h>
#include <errno.h>

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
	struct ifconf ifconf;
	char buf[1024] ={0};
	int len = 256;
	int sockfd = -1;
	int ret = -1;
	int i = 0;
	struct ifreq *ifreq = NULL;

	ifconf.ifc_buf = buf;
	ifconf.ifc_len = len;
	
	sockfd = init_socket();
	
	ret = ioctl(sockfd, SIOCGIFCONF, &ifconf);
	if (ret == -1)
	{
		printf("ioctl error, errno =%d\n", errno);
		return -1;
	}
	
	ifreq = (struct ifreq *)ifconf.ifc_buf;
	
	for (i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--)
	{
		printf("name = [%s]\n" , ifreq->ifr_name);
		printf("local addr = [%s]\n" ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
		printf("local addr = [%s]\n" ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr));
		ifreq++;
	}
	
	return 0;
}