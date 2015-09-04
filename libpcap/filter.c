#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <errno.h>

int pcap_filter(pcap_t *device, const char *buf)
{
	if ((NULL == device) || (NULL == buf))
	{
		printf("pcap_filter: input error\n");
		return -1;
	}

	struct bpf_program filter;

	if (pcap_compile(device, &filter, buf, 1, 0) == -1)
	{
		printf("pcap_compile error, errno = %d.\n", errno);
		return -1;
	}

	if (pcap_setfilter(device, &filter) == -1)
	{
		printf("pcap_setfilter error, errno = %d.\n", errno);
		return -1;	
	}

	return 0;
}



