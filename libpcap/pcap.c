#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "callback.h"
#include "filter.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("input num error\n");
		return -1;
	}
	
	char errBuf[PCAP_ERRBUF_SIZE];
	char *devStr = NULL;
	pcap_t *device = NULL;
	int id = 0;
	int cap_num = 0;

	memset(errBuf, 0, sizeof(errBuf));

	/* get a device */
	devStr = pcap_lookupdev(errBuf);
	if(NULL == devStr)
	{
		printf("error: %s\n", errBuf);
		exit(1);
	}

	/* open a device, wait until a packet arrives */
	device = pcap_open_live(devStr, 65535, 1, 0, errBuf);
	if(NULL == device)
	{
		printf("error: pcap_open_live(): %s\n", errBuf);
		exit(1);
	}

	printf("is net device support monitor mode, %s\n", pcap_can_set_rfmon(device) ? "yes" : "no");
	if (pcap_datalink(device) != DLT_EN10MB) 
	{
		printf("%s is not an Ethernet\n", devStr);
		return -1;
	}

	pcap_filter(device, "icmp");
	
	cap_num = atoi(argv[1]);
	pcap_loop(device, cap_num, printHex, (u_char*)&id);

	pcap_close(device);

	return 0;
}


