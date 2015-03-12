#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/filter.h>
#include <stdlib.h>
#include <errno.h>

#define ETH_HDR_LEN 14
#define IP_HDR_LEN 20
#define UDP_HDR_LEN 8
#define TCP_HDR_LEN 20

int main(int argc, char **argv)
{
    struct ifreq ifreq;
    struct sock_fprog filter;
    int sockfd = -1;
    int ret = -1;
    char buf[2048] = {0};
    int n = 0;
    char ethhead[100] = {0};
    char iphead[100] = {0};
    int i = 0;
    struct sockaddr_in *sin;
    
    struct sock_filter bpf[] = 
    {
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 0, 6, 0x000086dd },
        { 0x30, 0, 0, 0x00000014 },
        { 0x15, 0, 15, 0x00000006 },
        { 0x28, 0, 0, 0x00000036 },
        { 0x15, 12, 0, 0x00000050 },
        { 0x28, 0, 0, 0x00000038 },
        { 0x15, 10, 11, 0x00000050 },
        { 0x15, 0, 10, 0x00000800 },
        { 0x30, 0, 0, 0x00000017 },
        { 0x15, 0, 8, 0x00000006 },
        { 0x28, 0, 0, 0x00000014 },
        { 0x45, 6, 0, 0x00001fff },
        { 0xb1, 0, 0, 0x0000000e },
        { 0x48, 0, 0, 0x0000000e },
        { 0x15, 2, 0, 0x00000050 },
        { 0x48, 0, 0, 0x00000010 },
        { 0x15, 0, 1, 0x00000050 },
        { 0x6, 0, 0, 0x00040000 },
        { 0x6, 0, 0, 0x00000000 },
    };
    
    memset(&ifreq, 0, sizeof(ifreq));
    memset(&filter, 0, sizeof(filter));

    filter.len = sizeof(bpf) / sizeof(struct sock_filter);
    filter.filter = bpf;

    sockfd = socket(AF_PACKET, SOCK_RAW, htonl(ETH_P_IP));
    if (sockfd < 0)
    {
        printf("create socket error\n");
        return -1;
    }

    memcpy(ifreq.ifr_name, "eth1", IFNAMSIZ);

    ret = ioctl(sockfd, SIOCGIFFLAGS, &ifreq);
    if (ret < 0)
    {
        printf("ioctl get flags error\n");
        return -1;
    }

    ret = ioctl(sockfd, SIOCSIFFLAGS, &ifreq);
    if (ret < 0)
    {
        printf("ioctl set flags error\n");
        return -1;
    }
    ret = ioctl(sockfd, SIOCGIFADDR, &ifreq);
    if (ret == -1)
	{
		printf("ioctl error, errno =%d\n", errno);
		return -1;
	}
	sin = (struct sockaddr_in *)&ifreq.ifr_addr;
	printf("ip addr = %s\n", inet_ntoa(sin->sin_addr));
    
    ret = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
    if (ret < 0)
    {
        printf("socksetopt bpf filter error\n");
        return -1;
    }

    printf("start capture packet\n");
    for(i = 0; i < 10; i++)
    {
        printf("i = %d\n", i);

        memset(buf, 0, sizeof(buf));
        n = read(sockfd, buf, sizeof(buf));
        if(n < (ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN)) 
        {
            printf("recvfrom packet error\n");
        }

        printf("%d bytes recieved/n", n);

        memcpy(ethhead, buf, ETH_HDR_LEN);
        printf("Ethernet: MAC[%02X:%02X:%02X:%02X:%02X:%02X]", ethhead[0], ethhead[1], ethhead[2],
            ethhead[3], ethhead[4], ethhead[5]);
        printf("->[%02X:%02X:%02X:%02X:%02X:%02X]", ethhead[6], ethhead[7], ethhead[8],
            ethhead[9], ethhead[10], ethhead[11]);
        printf(" type[%04x]/n", (ntohs(ethhead[12]|ethhead[13]<<8)));

        memcpy(iphead, buf + ETH_HDR_LEN, IP_HDR_LEN);
        printf("IP: Version: %d HeaderLen: %d[%d]", (*iphead >> 4), (*iphead & 0x0f), (*iphead & 0x0f) * 4);
        printf(" TotalLen %d", (iphead[2] << 8 | iphead[3]));
        printf(" IP [%d.%d.%d.%d]", iphead[12], iphead[13], iphead[14], iphead[15]);
        printf("->[%d.%d.%d.%d]", iphead[16], iphead[17], iphead[18], iphead[19]);
        printf(" %d", iphead[9]);

        if(iphead[9] == IPPROTO_TCP)
            printf("[TCP]");
        else if(iphead[9] == IPPROTO_UDP)
            printf("[UDP]");
        else if(iphead[9] == IPPROTO_ICMP)
            printf("[ICMP]");
        else if(iphead[9] == IPPROTO_IGMP)
            printf("[IGMP]");
        else if(iphead[9] == IPPROTO_IGMP)
            printf("[IGMP]");
        else
            printf("[OTHERS]");
 
        printf(" PORT [%d]->[%d]/n", (iphead[20]<<8|iphead[21]), (iphead[22]<<8|iphead[23]));

        sleep(1);
    }

    strncpy(ifreq.ifr_name, "eth1", IFNAMSIZ);
    if(ioctl(sockfd, SIOCGIFFLAGS, &ifreq) != -1) 
    {
        ifreq.ifr_flags &= ~IFF_PROMISC;
        ioctl(sockfd, SIOCSIFFLAGS, &ifreq);
    }
    close(sockfd);

    return 0;
}