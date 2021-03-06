#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>

#include <stdint.h>
#include "filter.hpp"
#include "stat.h"

using namespace std;

#define LOGINNER(msg) cout << __FILE__ <<":"<<__LINE__<<"-"<<msg<<" "<<strerror(errno)<<endl;

#ifdef LOG
#   define LOGMSG(msg) cout << __FILE__ <<":"<<__LINE__<<"-"<<msg<<" "<<endl;
#else
#   define LOGMSG(msg) cout << msg<<endl;
#endif

#   define LOG(fmt,...) printf(fmt,__VA_ARGS__);

#define ETH_P_ALL 0x0003
typedef const string  CString;

enum PROTO_TYPE
{
    PROTO_TCP=1,
    PROTO_UDP=2,
    PROTO_ICMP=3
};

class CNetModelInterface
{
    public:
        CNetModelInterface(int32_t fd,string host="127.0.0.1",int32_t port=0,PROTO_TYPE type=PROTO_UDP):m_netSockFd(fd), \
                m_host(host),m_port(port),m_buffer(""),m_type(type)
        {
        }

    public:
        virtual int32_t Init(CFilter )=0;
        virtual int32_t NetHandleEvent()=0;
        virtual int32_t NetRecv()=0;
        virtual int32_t NetSend(CString& data)=0;

    protected:
        int32_t m_netSockFd;
        string  m_host;
        int32_t m_port;
        string  m_buffer;
        PROTO_TYPE m_type;
};

// Model should be select poll epoll
template<typename Model>
class CNet
{
    public:
        CNet()
        {
        }
        CNet(int32_t port,string host="127.0.0.1"):m_netSockFd(-1),m_netModel(NULL)
        {
            m_host = host;
            m_port = port;
        }
        virtual ~CNet()
        {
            if(m_netSockFd > 0)
                close(m_netSockFd);
            m_netSockFd=0;
            if(m_netModel != NULL)
            {
                delete m_netModel;
                m_netModel = NULL;
            }
        }
#if 1
        // 链路抓包,不指定端口则收取所有端口数据包,z支持接受包
        virtual int32_t InitNetRawSocket(CFilter filter,bool isLingger=false,bool isUnBlock=false,CString & ether="eth0")
        {
            struct ifreq ifr;

            memset(&ifr, 0x00,sizeof(struct ifreq));
            if((m_netSockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0 )
            {
                LOGINNER("ERROR:Create raw socket failed")
            }
            strncpy(ifr.ifr_name, ether.c_str(), ether.length());
            if(ioctl(m_netSockFd, SIOCGIFFLAGS,&ifr) == -1)
            {
                LOGINNER("ERROR:set SIOCGIFFLAGS,ioctl return failed");
                exit(errno);
            }

            ifr.ifr_flags |= IFF_PROMISC;
            if(ioctl(m_netSockFd, SIOCSIFFLAGS, & ifr) == -1)
            {
                LOGINNER("ERROR:set SIOCSIFFLAGS,ioctl return failed");
                exit(errno);
            }
            if(fcntl(m_netSockFd, F_SETFL, O_NONBLOCK) < 0)
            {
                LOGINNER("ERROR:set fd nonblock ,fcntl return failed");
                exit(errno);
            }
            if(m_netModel)
            {
                delete m_netModel;
                m_netModel = NULL;
            }
            m_netModel = new Model(m_netSockFd,m_host,m_port);
            if(m_netModel == NULL)
            {
                LOGINNER("ERROR:Create net Model failed");
                exit(errno);
            }
            m_netModel->Init(filter);
            m_netModel->NetHandleEvent();
        }
#endif
        //网络层抓包，不指定端口则收取所有端口数据包
        virtual int32_t InitNetServerSocket(CFilter filter,bool isLingger=false,bool isUnBlock=false)
        {
            m_netSockFd = socket(AF_INET, SOCK_RAW, htons(ETH_P_ALL));
            if(m_netSockFd == -1)
            {
                LOGINNER("ERROR:Create socket failed:");
                exit(errno);
            }
            fprintf(stderr,"create socket ok:%d\n",m_netSockFd);
            int iReuseaddr = 1;
            if(setsockopt(m_netSockFd,SOL_SOCKET,SO_REUSEADDR,(const void*)&iReuseaddr,sizeof(int)) == -1)
            {
                LOGINNER("ERROR:Set socket address reuse failed");
                exit(errno);
            }
            if(isUnBlock)
            {
                int ret = fcntl(m_netSockFd,F_SETFL,O_NONBLOCK);
                if (ret == -1)
                {
                    LOGINNER("ERROR:Set socket nonblock failed");
                    exit(errno);
                }
            }

            if(isLingger)
            {
                struct linger ling={0,5};
                if(setsockopt(m_netSockFd,SOL_SOCKET,SO_LINGER,(const char*)&ling,sizeof(bool)))
                {
                    LOGINNER("ERROR:Set socket linger failed");
                    exit(errno);
                }
            }

            if(m_netModel)
            {
                delete m_netModel;
                m_netModel = NULL;
            }
            m_netModel = new Model(m_netSockFd,m_host,m_port);
            if(m_netModel == NULL)
            {
                LOGINNER("ERROR:Create net Model failed");
                exit(errno);
            }
            m_netModel->Init(filter);
            m_netModel->NetHandleEvent();
        }
    private:
        int32_t m_netSockFd;
        Model  *m_netModel;
        string  m_host;
        int32_t m_port;

};




