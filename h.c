#include<stdio.h>
#include "prt.h"
int main(int argc, char **argv)
{
	printf("[%s]:[%s]:[%d], printf\n", __FILE__, __FUNCTION__, __LINE__);
	PRT("PRT%d\n", 1);
}

