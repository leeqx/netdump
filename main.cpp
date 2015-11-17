#include "net.hpp"
#include "myparse.hpp"
#include "select.hpp"
#include "filter.hpp"

int useage()
{
	fprintf(stdout,
			"usage:\n"
			"\t s-packet size \n"
			"\t S-source ip \n"
			"\t D-dest   ip \n"
			"\t P-port \n"
			"\t t-protocol type:tcp|udp|icmp \n"
			"\t i-ether  lo|eth0|eth1 \n"
			"\t c-max packet count \n");
	return 0;
}
int main(int argc,char* argv[])
{
	if(argc < 2)
	{
		useage();
		exit(-1);
	}
	int c = 0;
	extern int optind,optopt;
	extern char* optarg;
	CFilter filter;
	// s-packet size
	// S-source ip
	// D-dest   ip
	// P-port
	// t-protocol type:tcp|udp|icmp
	// i-ether  lo|eth0|eth1
	// c-max packet count
	// 
	while((c=getopt(argc,argv,"s:S:D:P:t:i:c:")) != -1)
	{
		switch(c)
		{
			case 's':
			{	
				filter.SetPacketSize(atoi(optarg));
				break;
			}
			case 'S':
			{
				filter.SetSrcIp(optarg);
				break;
			}
			case 'D':
			{
				filter.SetDstIp(optarg);
				break;
			}
			case 'P':
			{
				filter.SetSrcPort(atoi(optarg));
				break;
			}
			case 't':
			{
				if(strncmp(optarg,"tcp",strlen("tcp"))==0)
					filter.SetProtoType(0);
				else if(strncmp(optarg,"udp",strlen("udp")) == 0)
					filter.SetProtoType(1);
				else
				{
					fprintf(stderr,"prototype %s unsuport this version\n",optarg);
					exit(0);
				}
				break;
			}
			case 'i':
			{
				if(strncmp(optarg,"any",strlen("any")) ==0)
				{
					fprintf(stderr,"current version unsport ether type any\n");
					exit(0);
				}	
                else
                    filter.SetEtherType(optarg);
				break;
			}
			case 'c':
			{
				filter.SetPacketCount(atoi(optarg));
				break;
			}
			default:
			{
				fprintf(stderr,"unsuport option\n");
				useage();
				break;
			}
		}
	}


	CNet<CSelect<CMyParse> > mydump(filter.GetSrcPort());
	//mydump.InitNetServerSocket(false,true);
	mydump.InitNetRawSocket(filter,false,true,filter.GetEtherType());
}

