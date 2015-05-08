#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netpacket/packet.h>

#define MAC_BCAST_ADDR		(uint8_t *) "\xff\xff\xff\xff\xff\xff"
#define LOCAL_INTERFACE		"eth0"
#define ARPOP_REQUEST		1
#define ARPOP_REPLY			2
#define u_char unsigned		char
#define u_short unsigned	short
#define u_int unsigned		int

struct arpMsg
{
	/* Ethernet header */
	u_char   h_dest[6];			/* destination ether addr */
	u_char   h_source[6];		/* source ether addr */
	u_short  h_proto;			/* packet type ID field */

	/* ARP packet */
	u_short htype;				/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;				/* protocol type (must be ETH_P_IP) */
	u_char  hlen;				/* hardware address length (must be 6) */
	u_char  plen;				/* protocol address length (must be 4) */
	u_short operation;			/* ARP opcode */
	u_char  sHaddr[6];			/* sender's hardware address */
	u_char  sInaddr[4];			/* sender's IP address */
	u_char  tHaddr[6];			/* target's hardware address */
	u_char  tInaddr[4];			/* target's IP address */
	u_char  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
} arp_pack;

//mac = my mac
static int recv_arp_response(int sock, u_int target_ip, u_int src_ip, u_char *src_mac, u_char *target_mac)
{
	int ret = 0;
	struct arpMsg arp;
	fd_set fdset;
	struct timeval tm;
	int time = 4;

	void *p_src_ip = NULL;
	void *p_tar_ip = NULL;

	memset(&arp, 0, sizeof(struct arpMsg));
	tm.tv_sec = 1;
	tm.tv_usec = 0;

	/* wait arp reply, and check it */
	while (time)
	{
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		ret = select(sock + 1, &fdset, NULL, NULL, &tm);
		if (ret < 0)
		{
			printf("Error recv the arp response error=%d\n", errno);
			continue;
		}
		else if (0 == ret)
		{
			printf("select time out !error=%d\n", errno);
			continue;
		}
		else
		{
			if (FD_ISSET(sock, &fdset))
			{
				if (recv(sock, &arp, sizeof(arp), 0) < 0)
				{
					printf("recv error error = %d\n", errno);
					continue;
				}
				
				//Ip address is a integer variable, but in struct arpMsg, it was defined as 4 character words.
				//Need to be cast to void pointer, then compare with the src_ip or target_ip
				p_src_ip = (void *)(arp.sInaddr);   
				p_tar_ip = (void *)(arp.tInaddr);

				if ((htons(ARPOP_REPLY) == arp.operation) && (memcmp(arp.tHaddr, src_mac, 6) == 0))
				{
					if ((memcmp(p_tar_ip, (void *)&src_ip, 4) == 0)
						 && (memcmp(p_src_ip, (void *)&target_ip, 4) == 0))
					{
						memcpy(target_mac, arp.sHaddr, sizeof(arp.sHaddr));
						return 0;
					}
				}
			}
		}
		sleep(1);
		time--;
	}

	if(time == 0)
	{
		printf("The destination address can not reachable\n");
		return -1;
	}
	return 0;
}

int get_iface_index(int fd, const char* interface_name)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strcpy (ifr.ifr_name, interface_name);
	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
	{
		return (-1);
	}
	
	return ifr.ifr_ifindex;
}

int send_arp_request(int sock, u_int target_ip, u_int src_ip, u_char *dst_mac, u_char *src_mac, char *interface_name)
{
	if ((NULL == dst_mac) || (NULL == src_mac) || (NULL == interface_name))
	{
		return -1;
	}
	
	struct sockaddr_ll addr;
	struct arpMsg arp;

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = get_iface_index(sock, interface_name);
	addr.sll_protocol = htons(ETH_P_ARP);

	/* construct arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.h_dest, dst_mac, 6);  /* MAC DA */
	memcpy(arp.h_source, src_mac, 6);       /* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);         /* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);        /* hardware type */
	arp.ptype = htons(ETH_P_IP);            /* protocol type (ARP message) */
	arp.hlen = 6;                           /* hardware address length */
	arp.plen = 4;                           /* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);   /* ARP op code */
	memcpy(arp.sInaddr, &src_ip, sizeof(src_ip)); /* source IP address */
	memcpy(arp.sHaddr, src_mac, 6);         /* source hardware address */
	memcpy(arp.tInaddr, &target_ip, sizeof(target_ip)); /* target IP address */

	if (sendto(sock, &arp, sizeof(arp), 0, (struct sockaddr*)&addr, sizeof(addr)) != sizeof(arp))
	{
		printf("Could not send arp package errno=%d\n", errno);
		return -1;
	}

	return 0;
}

int get_target_mac(u_int target_ip, u_int src_ip, u_char *src_mac, u_char *target_mac, char *interface)
{
	int optval = 1;
	int s = -1;

	s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));  //create 
	if (-1 == s)
	{
		printf("Could not open raw socket");
		return -1;
	}
	
/*	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));   //PROMISC mode
	strcpy(ifr.ifr_name, "eth0");
	if ((ioctl(s, SIOCGIFFLAGS, &ifr)) < 0) 
	{
		printf("ioctl SIOCGIFFLAGS error\n");
		close(s);
		return-1;
	}

	ifr.ifr_flags |= IFF_PROMISC;
	if ((ioctl(s, SIOCSIFFLAGS, &ifr)) < 0)
	{
		printf("ioctl SIOCSIFFLAGS error\n");
		return -1;
	}
*/
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1)  //set socket broadcast
	{
		printf("Could not setsocketopt on raw socket");
		close(s);
		return -1;
	}

	if (send_arp_request(s, target_ip, src_ip, MAC_BCAST_ADDR, src_mac, interface) < 0)
	{
		printf("arp_ping: sendto failed, errno=%d\n", errno);
		close(s);
		return -1;
	}
	//printf("arp_ping: sendto success\n");

	if (recv_arp_response(s, target_ip, src_ip, src_mac, target_mac) < 0)
	{
		printf("arp_ping: recv failed, errno=%d\n", errno);
		close(s);
		return -1;
	}
	//printf("arp_ping: recv success\n");

	close(s);
	return 0;
}

int get_ipaddr_macaddr(char *interface_name, u_char *mac)
{
	if (NULL == interface_name)
	{
		printf("interface_name error!\n");
		return -1;
	}

	int s = -1;
	struct ifreq ifr;
	struct sockaddr_in *ptr = NULL;
	u_int ip = 0;

	memset(&ifr, 0, sizeof(ifr));

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	strncpy(ifr.ifr_name, interface_name, strlen(interface_name));

	if(ioctl(s, SIOCGIFADDR, &ifr) < 0)
	{
		printf("get_ipaddr ioctl error and errno=%d\n", errno);
		close(s);
		return -1;
	}

	ptr = (struct sockaddr_in *)&ifr.ifr_addr;
	ip = ptr->sin_addr.s_addr;

	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
	{
		//SIOCGIFHWADDR get hardware address
		printf("get_macaddr ioctl error and errno=%d\n", errno);
		close(s);
		return -1;
	}

	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

	close(s);

	return ip;
}

int main(int argc, char **argv)
{
	u_int my_ip = 0;
	u_int target_ip = 0;
	int oc = 0; //input operation

	u_char my_mac[6];
	u_char target_mac[6];
	char interface_name[5];
	u_char buf[32];

	memset(interface_name, 0, sizeof(interface_name));
	memset(buf, 0, sizeof(buf));
	memset(my_mac, 0, sizeof(my_mac));
	memset(target_mac, 0, sizeof(target_mac));

	while((oc = getopt(argc, argv, "i:t:")) != -1)
	{
		switch(oc)
		{
		case 'i':
			memcpy(interface_name, optarg, 4);
			break;
		case 't':
			memcpy(buf, optarg, strlen(optarg));
			target_ip = inet_addr((const char *)buf);
			break;
		default:
			printf("other option:%c\n", oc);
			break;
		}
	}

	my_ip = get_ipaddr_macaddr(interface_name, my_mac);
	if (get_target_mac(target_ip, my_ip, my_mac, target_mac, interface_name) < 0)
	{
		printf("can not get the target macaddr\n");
		return -1;
	}

	printf("target_mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",    // mac can not print use %s
			target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);

	return 0;
}

