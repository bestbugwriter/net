#include <libnet.h>

int main(int argc, char **argv)
{
	libnet_t *l;
	char err_buf[LIBNET_ERRBUF_SIZE];
	char *device = "wlan0";
	unsigned int srcIp = 0;
	unsigned int dstIp = 0;
	struct libnet_ether_addr *srcMac;
	struct libnet_ether_addr dstMac;
	libnet_ptag_t ptag;
	uint8_t *packet;
	uint32_t packet_s;
	
	l = libnet_init(LIBNET_LINK_ADV, device, err_buf);
	if (l == NULL)
	{
		printf("%s\n", err_buf);
		exit(-1);
	}

	srcIp = libnet_get_ipaddr4(l);
	srcMac = libnet_get_hwaddr(l);

	dstIp = libnet_name2addr4(l, "192.168.0.6", 0);
	ptag = libnet_build_arp(ARPHRD_ETHER, ETHERTYPE_IP, 6, 4, ARPOP_REPLY, srcMac->ether_addr_octet, 
		(u_int8_t *)&srcIp, dstMac.ether_addr_octet, (u_int8_t *)&dstIp, NULL, 0, l, 0);

	if (ptag == -1)
	{
		printf("%s\n", libnet_geterror(l));
		libnet_destroy(l);
		exit(-1);
	}

	ptag = libnet_autobuild_ethernet((u_int8_t *)dstMac.ether_addr_octet, ETHERTYPE_ARP, l);
	if (ptag == -1)
	{
		printf("%s\n", libnet_geterror(l));
		libnet_destroy(l);
		exit(-1);
	}

	if (libnet_adv_cull_packet(l, &packet, &packet_s) == -1)
	{
		fprintf(stderr, "%s", libnet_geterror(l));
		libnet_destroy(l);
	}
	else
	{
		fprintf(stderr, "packet size: %d\n", packet_s);
		libnet_adv_free_packet(l, packet);
	}

	if (libnet_write(l) == -1)
	{
		printf("%s\n", libnet_geterror(l));
		libnet_destroy(l);
		exit(-1);
	}

	libnet_destroy(l);

	return 0;
}
