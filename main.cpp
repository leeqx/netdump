#include "net.hpp"
#include "myparse.hpp"
#include "select.hpp"
#include "filter.hpp"
#include "redis_parse.hpp"

enum EParseTpe
{
    EDEFAULT=0,
    EREDIS=1,
    EPROTOBUFER=2
};
int useage()
{
    fprintf(stdout,
     "usage:\n"
     "\t s-packet size \n"
     "\t S-source ip \n"
     "\t D-dest   ip \n"
     "\t p-source and dst port\n"
     "\t P-source port\n"
     "\t d-dst    port\n"
     "\t t-protocol type:tcp|udp|icmp \n"
     "\t i-ether  lo|eth0|eth1 \n"
     "\t c-max packet count \n"
     "\t u-use parse to pare packet[raw|redis]\n");
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
    int parseType = 0;
    extern char* optarg;
    CFilter filter;
    // s-packet size
    // S-source ip
    // D-dest   ip
    // p-source port
    // d-dst    port
    // t-protocol type:tcp|udp|icmp
    // i-ether  lo|eth0|eth1
    // c-max packet count
    // u-use parse to pare packet
    //
    while((c=getopt(argc,argv,"s:S:D:t:i:c:p:d:u:P:")) != -1)
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
            case 'p':
                {
                    filter.SetBothPort(atoi(optarg));
                    break;
                }
            case 'P':
                {
                    filter.SetSrcPort(atoi(optarg));
                    break;
                }
            case 'd':
                {
                    filter.SetDstPort(atoi(optarg));
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
            case 'u':
            case 'U':
                {
                    if(strncmp(optarg,"raw",strlen("raw")) == 0)
                    {
                        fprintf(stdout,"set parser as raw type\n");
                        parseType = EDEFAULT;
                    }
                    else if(strncmp(optarg,"redis",strlen("redis")) == 0)
                    {
                        fprintf(stdout,"set parser as REDIS protocol type\n");
                        parseType = EREDIS;
                    }
                    else
                    {
                        fprintf(stdout,"set parser as raw type:%s\n",optarg);
                        parseType = EDEFAULT;
                    }
                    break;
                }
            default:
                {
                    fprintf(stderr,"unsuport option:%s\n",optarg);
                    useage();
                    return 0;
                }
        }
    }

    switch(parseType)
    {
        case EREDIS:
            {
                CNet<CSelect<CRedisParse> > mydump(filter.GetSrcPort());
                mydump.InitNetRawSocket(filter,false,true,filter.GetEtherType());
                break;
            }
        case EDEFAULT:
        default:
            {
                CNet<CSelect<CMyParse> > mydump(filter.GetSrcPort());
                mydump.InitNetRawSocket(filter,false,true,filter.GetEtherType());
                break;
            }

    }
    return 0;
}

