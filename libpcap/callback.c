#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "callback.h"

void printHex(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	if ((NULL == arg) || (NULL == pkthdr) || (NULL == packet))
	{
		printf("getPacket: input error\n");
		return;
	}
	
	int *id = (int *)arg;
	int i = 0;
	
	printf("id: %d\n", ++(*id));
	printf("Packet length: %d\n", pkthdr->len);
	printf("Number of bytes: %d\n", pkthdr->caplen);
	printf("Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec)); 

	for(i = 0; i < pkthdr->len; ++i)
	{
		printf(" %02x", packet[i]);
		if((i + 1) % 16 == 0)
		{
			printf("\n");
		}
	}

	printf("\n\n");
}



