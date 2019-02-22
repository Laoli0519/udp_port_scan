#include "udpPortScan.h"



//每秒钟应该发包数量
int speed_package_number = 0;

//正在扫描的端口
int port_number;

struct optparse_long long_option[] = {
	{ "help", 'h', OPTPARSE_NONE },
	{ "ip", 'p', OPTPARSE_REQUIRED },
	{ "minport", 'i', OPTPARSE_REQUIRED },
	{ "maxport", 'a', OPTPARSE_REQUIRED },
	{ "bandwidth", 'b', OPTPARSE_REQUIRED },
	{ 0 }
};

void Usage()
{
	printf("\n----------Port_test_usage-----------\n\n");
	printf("--help(-h)                      : Provide help information\n");
	printf("--ip(-p)    [example:127.0.0.1] : Scan the server address\n");
	printf("--minport(-i)   [example:10000] : The minimum value of the scanned port address\n");
	printf("--maxport(-a)   [example:20000] : The maximum value of the scanned port address\n");
	printf("--bandwidth(-b) [example:2 m/s] : Specifies the bandwidth to be used\n\n");
	printf("------------------------------------\n\n");
	printf("A complete instance : -p 127.0.0.1 -i 10000 -a 20000 -b 2\n\n");
	printf("------------------------------------\n");
}

//初始化
void Initialize(parameter *arg)
{
	if (NULL == arg)
	{
		fprintf(stderr, "Error : Initialize : Use null pointer!\n");
		exit(1);
	}

	Check_the_parameters(arg);

	speed_package_number = Compute_send_packet_speed(arg->bandwidth);
}

void Parse_input_parameter(int argc, char *argv[], parameter *arg)
{
	if (!(argv && argv[0] && arg))
	{
		fprintf(stderr, "Error : Parse_input_parameter : Use null pointer!\n");
		exit(1);
	}

	int opt = 0;
	struct optparse options;

	optparse_init(&options, argv);
	while ((opt = optparse_long(&options, long_option, NULL)) != -1)
	{
		switch (opt)
		{
			case 'p': strcpy(arg->ip, options.optarg);
					  break;
			case 'i': arg->minport = atoi(options.optarg);
					  break;
			case 'a': arg->maxport = atoi(options.optarg);
					  break;
			case 'b': arg->bandwidth = atoi(options.optarg);
					  break;
			case 'h': Usage();
					  break;
		}
	}
}

//检查ip地址是点分十进制还是域名
static int Check_ip_format(const char *ip)
{
	for (int i = 0; i < 50; ++i)
	{
		if (ip[i] >= 'a' && ip[i] <= 'z' || ip[i] >= 'A' && ip[i] <= 'Z')
			return 0;
	}
	return 1;
}


//简单检查参数是否正确
void Check_the_parameters(parameter *arg)
{
	if (NULL == arg)
	{
		fprintf(stderr, "Error : Check_the_parameters : Use null pointer!\n");
		exit(1);
	}

	int ret = 0;

	//ip_address
	if (0 == arg->ip[0])
	{
		fprintf(stderr, "Error : Lack of parameter : IP\n"), ret = 1;
	}
	else if (Check_ip_format(arg->ip))
	{
		if (-1 == inet_addr(arg->ip))
		{
			fprintf(stderr, "Error : Wrong parameter : IP\n"), ret = 1;
		}
	}
	else
	{
		Convert_IP_address(arg);
	}

	//port : 1 - 65535
	if (-1 == arg->minport || -1 == arg->maxport)
	{
		fprintf(stderr, "Error : Lack of parameter : port_range\n"), ret = 1;
	}
	else if (arg->minport <= 0 || arg->maxport >= 65535)
	{
		fprintf(stderr, "Error : Wrong parameter : port\n"), ret = 1;
	}

	if (arg->minport > arg->maxport)
	{
		int temp = arg->minport;
		arg->minport = arg->maxport;
		arg->maxport = temp;
	}

	//bandwidth
	if (0 >= arg->bandwidth)
	{
		fprintf(stderr, "Error : Wrong parameter : bandwidth\n"), ret = 1;
	}

	if (ret)
	{
		Destroy(arg, NULL, NULL);
		exit(6);
	}
}

int Compute_send_packet_speed(int bandwidth)
{
	return bandwidth * 1024 / 5;
}

void Convert_IP_address(parameter *arg)
{
	if (NULL == arg)
	{
		fprintf(stderr, "Error : Convert_IP_address : Use null pointer!\n");
		exit(1);
	}

	
	if (inet_addr(arg->ip) == -1)
	{
		fprintf(stderr, "Convert_IP_address : InetNtop");
		Destroy(arg, NULL, NULL);
		exit(5);
	}
}

//扫描端口逻辑控制函数
void Scanning_ports(parameter *arg)
{
	if (NULL == arg)
	{
		fprintf(stderr, "Error : Scanning_ports : Use null pointer!\n");
		exit(1);
	}

	//初始化

	int sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	
	if (sock_udp < 0)
	{
		perror("socket");
		Destroy(arg, NULL, NULL);
		exit(11);
	}

	struct sockaddr_in addr_server = { 0 };									//服务器的地址数据结构
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = inet_addr(arg->ip);

	struct timeval nNetTimeout = {3, 0};
	if(-1 == setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout, sizeof(struct timeval)) )
	{
		perror("setsockopt failed");
		Destroy(arg, NULL, &sock_udp);
		exit(10);
	}

	conclusion_t* cclu = (conclusion_t *)calloc(1, sizeof(conclusion_t));
	if (NULL == cclu)
	{
		perror("Scanning_ports : calloc");
		Destroy(arg, NULL, &sock_udp);
		exit(1);
	}

	cclu->port_list = (char *)calloc(arg->maxport - arg->minport + 1, sizeof(char));
	if (NULL == cclu->port_list)
	{
		perror("Scanning_ports : calloc");
		Destroy(arg, cclu, &sock_udp);
		exit(1);
	}

	gettimeofday(&cclu->start_scan, NULL);

	for (int port_number = arg->minport; port_number <= arg->maxport; ++port_number)
	{
		char send_buffer[BUFFER_SIZE];
		char recv_buffer[BUFFER_SIZE];
		int len;

		memset(send_buffer, 1, BUFFER_SIZE);

		addr_server.sin_port = htons(port_number);	//将目的端口号装入
	
		sprintf(send_buffer, "%d", port_number);	

		//控制发包速度
		if ((port_number - arg->minport + 1) % speed_package_number == 0)
		{
			sleep(1);
		}

		for (int i = 0; i < PORT_PACKAGE_NUMBER; ++i)
		{
			if ((sendto(sock_udp, send_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr_server, sizeof(struct sockaddr))) < 0)
			{
				perror("sendto");
				continue;
			}
		}

		for (int i = 0; i < PORT_PACKAGE_NUMBER; ++i)
		{
			memset(recv_buffer, 0, BUFFER_SIZE);
			len = sizeof(struct sockaddr);
			int temp;

			if (recvfrom(sock_udp, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr_server, &len) < 0)
			{
				break;
			}
			temp = atoi(recv_buffer);
			//printf("temp = %d\n", temp);
			if (temp >= arg->minport)
			{
				++(cclu->port_list[temp - (arg->minport)]);
			}
		}
		if (cclu->port_list[port_number - (arg->minport)] > PORT_PACKAGE_NUMBER / 2)
		{
			printf("scaning port = %d .. open\n", port_number);
		}
		else
		{
			printf("scaning port = %d .. closed\n", port_number);
		}
	}
	
	

	gettimeofday(&cclu->end_scan, NULL);

	Conclusion(arg, cclu);

	Destroy(arg, cclu, &sock_udp);
}


//清理函数
void Destroy(parameter *arg, conclusion_t * cclu, int *sock_udp)
{
	if (NULL != sock_udp)
	{
		close(*sock_udp);
	}

	if (NULL != arg)
	{
		free(arg);
	}

	if (NULL != cclu)
	{
		free(cclu->port_list);
		cclu->port_list = NULL;
		free(cclu);
	}
}

void Conclusion(parameter *arg, conclusion_t * cclu)
{
	if (NULL == arg || NULL == cclu)
	{
		fprintf(stderr, "Error : Conclusion : Use null pointer!\n");
		return;
	}

	printf("------------------------Closed_Port_List-----------------------\n");
	int scan_port_number = arg->maxport - arg->minport + 1;

	for (int i = 0; i <= scan_port_number / 10; ++i)
	{
		for (int j = 0; (i * 10 + j < scan_port_number) && (j < 10); ++j)
		{
			if (cclu->port_list[i * 10 + j] <= PORT_PACKAGE_NUMBER / 2)
			{
				printf("%5d  ", i * 10 + j + arg->minport);
				++(cclu->closed_port_number);
			}
		}
		printf("\n");
	}

	double spend_time = cclu->end_scan.tv_sec - cclu->start_scan.tv_sec + (cclu->end_scan.tv_usec - cclu->start_scan.tv_usec) / 1000000 ;

	printf("------------------------Scan_Conclusion------------------------\n");
	printf("Scanned ip address : %s\n", arg->ip);
	printf("Port range of this scan: %d - %d, Number of scanning ports : %d\n", arg->minport, arg->maxport, scan_port_number);
	printf("Number of closed ports : %d\n", cclu->closed_port_number);
	printf("Time spent scanning the port : %f s\n", spend_time);
}


void Run(int argc, char *argv[])
{
	parameter *arg = (parameter *)calloc(1, sizeof(parameter));
	if (NULL == arg)
	{
		perror("Run : malloc");
		exit(1);
	}

	arg->minport	= -1;
	arg->maxport	= -1;
	arg->bandwidth	= -1;

	Parse_input_parameter(argc, argv, arg);
	Initialize(arg);
	Scanning_ports(arg);
}
