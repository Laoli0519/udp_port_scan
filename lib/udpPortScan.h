#ifndef __UDPPORTSCAN_H__
#define __UDPPORTSCAN_H__


#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API				static
#include "optparse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct parameter
{
	char ip[50];
	int minport;
	int maxport;
	int bandwidth;
}parameter;


typedef struct conclusion
{
	int closed_port_number ;			//被关闭端口的数量
	char *port_list;
	struct timeval start_scan ;			//开始扫描时间
	struct timeval end_scan ;			//结束扫描时间
}conclusion_t;


#define BUFFER_SIZE					1024		//buffer_size
#define PORT_PACKAGE_NUMBER			5			//每个端口发包数量

void Usage();

void Initialize(parameter *arg);

void Parse_input_parameter(int argc, char *argv[], parameter *arg);

void Check_the_parameters( parameter *arg);

int Compute_send_packet_speed(int bandwidth);

void Convert_IP_address( parameter *arg);

void Scanning_ports(parameter *arg);

void Destroy(parameter *arg, conclusion_t *cclu, int *sock_udp);

void Conclusion( parameter *arg, conclusion_t *cclu);

void Run(int argc, char *argv[]);


#endif //__UDPPORTSCAN_H__

