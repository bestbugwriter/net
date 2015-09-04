#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "callback.h"
#include "filter.h"

int getInputParm(int argc, char **argv, char *filter_cond, int *num)
{
	if ((argc < 2) || (NULL == argv) || (NULL == filter_cond) || (NULL == num))
	{
		printf("getInputParm: input error\n");
		return -1;
	}
	
	int ch = 0;

	while((ch=getopt(argc, argv, "n:f:"))!=-1)
	{
		switch(ch)
		{
			case 'n':
				*num = atoi(optarg);
				break;

			case 'f':
				memcpy(filter_cond, optarg, strlen(optarg));
				break;

			case 'h':
				printf("-n capture packet number.\n-f filter condition\n");
				break;

			default:
				printf("-n capture packet number.\n-f filter condition\n");
				break;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	char errBuf[PCAP_ERRBUF_SIZE];
	char *devStr = NULL;
	pcap_t *device = NULL;
	int id = 0;
	int cap_num = 0;
	char filter_cond[1024];

	memset(filter_cond, 0, sizeof(filter_cond));
	memset(errBuf, 0, sizeof(errBuf));

	getInputParm(argc, argv, filter_cond, &cap_num);
	
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

	pcap_filter(device, filter_cond);
	
	pcap_loop(device, cap_num, printHex, (u_char*)&id);

	pcap_close(device);

	return 0;
}


