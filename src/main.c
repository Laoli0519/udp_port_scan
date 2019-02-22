#include "udpPortScan.h"


int main(int argc, char *argv[])
{
	if(1 == argc)
	{
		Usage();
		return 0;
	}

	Run(argc, argv);

	return 0;
}

