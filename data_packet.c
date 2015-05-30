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
#include <linux/types.h>
#include <linux/elf.h>
#include "getmac.h"

#define __LITTLE_ENDIAN_BITFIELD

#define EPT_IP   0x0800    /* type: IP */
#define EPT_ARP   0x0806    /* type: ARP */
#define EPT_RARP 0x8035    /* type: RARP */
#define ARP_HARDWARE 0x0001    /* Dummy type for 802.3 frames */
#define ARP_REQUEST 0x0001    /* ARP request */
#define ARP_REPLY 0x0002    /* ARP reply */

#define uchar unsigned		char
#define ushort unsigned		short
#define uint unsigned		int

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	__u8 mac_src[6];
	__u8 mac_dst[6];
	__u16 type;
}ETH_HEADER;

typedef struct
{
	__u16 hw_type;
	__u16 protocol_type;
	__u8 hw_len;
	__u8 protocol_len;
	__u16 op;
	__u8 mac_src[6];
	__u8 ip_src[4];
	__u8 mac_dst[6];
	__u8 ip_dst[4];
}ARP_HEADER;

typedef struct 
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8    ihl:4,
			version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8    version:4,
			ihl:4;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	__u8    tos;
	__be16  tot_len;
	__be16  id;
	__be16  frag_off;
	__u8    ttl;
	__u8    protocol;
	__be16  check;
	__be32  saddr;
	__be32  daddr;
}IP_HEADER;

typedef struct 
{
	__be16 source;
	__be16 dest;
	__be32 seq;
	__be32 ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16   res1:4,
			doff:4,
			fin:1,
			syn:1,
			rst:1,
			psh:1,
			ack:1,
			urg:1,
			ece:1,
			cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16   doff:4,
			res1:4,
			cwr:1,
			ece:1,
			urg:1,
			ack:1,
			psh:1,
			rst:1,
			syn:1,
			fin:1;
#else
#error "Adjust your defines"
#endif
	__be16 window;
	__be16 check;
	__be16 urg_ptr;
}TCP_HEADER;

typedef struct
{
	__u16 source;
	__u16 dest;
	__u16 len;
	__u16 check;
}UDP_HEADER;
#pragma pack(pop)

int make_eth_header(__u8 *mac_src, __u8 *mac_dst, __u16 protocol, ETH_HEADER *eth_header)
{
	if ((NULL == mac_src) || (NULL == mac_dst) || (0 == protocol) || (NULL == eth_header))
	{
		printf("make_eth_header: input error.\n");
		return -1;
	}
	
	memcpy(eth_header.mac_src, mac_src, strlen(mac_src));
	memcpy(eth_header.mac_dst, mac_dst, strlen(mac_dst));
	memcpy(eth_header.type, protocol, sizeof(protocol));
	
	return 0;
}

int make_arp_header(__u16 op, __u8 *mac_src, __u8 *ip_src, __u8 *mac_dst, __u8 *ip_dst, ARP_HEADER *arp)
{
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
	
}

int main(int argc, char **argv)
{
	printf("sizeof ip = %lu, arp = %lu, eth = %lu, tcp = %lu, udp = %lu\n", 
	sizeof(IP_HEADER), sizeof(ARP_HEADER), sizeof(ETH_HEADER), sizeof(TCP_HEADER), sizeof(UDP_HEADER));
	
	if (argc < 2)
	{
		printf("input param too few.\n");
		return -1;
	}
	
	int op;
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
	return 0;
}