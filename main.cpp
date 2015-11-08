#include "net.hpp"
#include "myparse.hpp"
#include "select.hpp"

int main(int argc,char* argv[])
{
	if(argc < 4)
	{
		printf("usage:%s port type[0-tcp|1-udp] ether[eth0|lo]\n",argv[0]);
		exit(-1);
	}

	CNet< CSelect<CMyParse> > mydump(atoi(argv[1]));
	//mydump.InitNetServerSocket(false,true);
	mydump.InitNetRawSocket(false,true,argv[3]);
}