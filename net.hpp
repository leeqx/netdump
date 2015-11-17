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

/**
 * 解析原始数据包：依次解析以太网数据包-ip包-tcp|UDP|ICMP数据包，
 * 提取最终的数据，更新到m_buffer中。
 * Note:所有自行定义的解析包需要集成自该类，否则需要在自己的解析类
 * 中自行解析以上三类协议报文头。
 */
class CNetPacketParse
{
	public:
		CNetPacketParse()
		{
		}
		CNetPacketParse(char* buffer,int len,CFilter filter):m_pEtherHdr(NULL),
										  	m_pIpHdr(NULL),
											m_pTcpHdr(NULL),
											m_pUdpHdr(NULL),
											m_state(0),
                                            m_filter(filter),
                                            m_msgType(-1)
                                                    
		{
            if( m_filter.GetPacketCount() != 0 && (m_filter.GetProtoType() == 0 && 
                 m_filter.GetPacketCount() == m_stat.m_netstat["tcp"]) ||
                (m_filter.GetProtoType() == 1 && 
                 m_filter.GetPacketCount() == m_stat.m_netstat["udp"]))
            {
                m_stat.dump();
                exit(0);
            }
            m_stat.m_netstat["all"]++;
			memset(m_rawBuffer,0x00,sizeof(m_rawBuffer));
			memset(m_buffer,0x00,sizeof(m_buffer));
			memcpy(m_rawBuffer,buffer,len);
			memcpy(m_buffer , buffer,len);

			m_bufferLen = len;
			m_rawBufferLen=len;

			char* tmp = this->ParseEtherHdr(m_buffer);
			tmp = this->ParseIpHdr(tmp);
			if(0 != m_state)
				return;

			char szSourceIp[32] = {0};
			char szDstIp[32] = {0};
			strcpy(szSourceIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->saddr)));
			strcpy(szDstIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->daddr)));


			switch(m_pIpHdr->protocol)
			{
				case IPPROTO_TCP:
                {
                    m_stat.m_netstat["tcp"]++;
                    m_msgType = 0;
                    tmp=this->ParseTcpHdr(tmp);
                    break;
                }
				case IPPROTO_UDP:
                {
                    m_stat.m_netstat["udp"]++;
                    m_msgType = 1;
                    tmp = this->ParseUdpHdr(tmp);
                    break;
                }
				case IPPROTO_ICMP:
                {
                    m_stat.m_netstat["icmp"]++;
                    m_msgType = 2;
                    break;
                }
				case IPPROTO_IGMP:
				case IPPROTO_RAW:
					break;
				default:
					LOGMSG("parse header,unknow prototcol");
					break;
			}
			memcpy(m_buffer ,(char*)tmp,m_bufferLen);

			this->DumpHeadInfo();
		}
		virtual int32_t Parse()=0;
		const char* GetBuffer(){return m_buffer;}
		const char* GetRawBuffer(){return m_rawBuffer;}
		const int32_t   GetBufferLen(){return m_bufferLen;}
		const int32_t   GetRawBufferLen(){return m_rawBufferLen;}
		
		char* ParseEtherHdr(char* pBuf)
		{
			if(NULL == pBuf || m_bufferLen <46) 
			{
				m_state = -1;
				LOGMSG("Parse ether_header,buffer is invalid");
				fprintf(stderr,"m_bufferLen=%u \n",m_bufferLen);
				return NULL;
			}
			m_state = 0;
			m_pEtherHdr = NULL;
			char *pTmp  = pBuf;
			m_pEtherHdr = (struct ether_header*) pTmp;
			//LOGMSG("Parse Ether_header ok");
			m_bufferLen -= sizeof(struct ether_header);
			return (char*)(pTmp + sizeof(struct ether_header));
		} 
		/** 
		 * 解析ip头部 
		 */ 
		char* ParseIpHdr(char* pBuf)
		{ 
			if(NULL == pBuf || m_bufferLen < 20 || m_state) {
				m_state = -2;
				LOGMSG("Parse iphdr,buffer is invalid");
				return NULL;
			}
			m_state=0;
			char* pTmp = pBuf;
			m_pIpHdr = NULL;
			m_pIpHdr = (struct iphdr*)(pTmp);

			// move forward to point tcp/udp/icmp header
			pTmp += (m_pIpHdr->ihl * 4);	
			m_bufferLen -= (m_pIpHdr->ihl *4);

			//LOGMSG("Parse iphdr ok");
			return (char*)pTmp;
		}

		/**
		 * 解析tcp头部:
		 * @note:
		 * tcp头部是变长的，所以需要注意
		 */	

		char* ParseTcpHdr(char *pBuf)
		{
			if(NULL == pBuf || m_bufferLen <20 || m_state)
			{
				m_state = -3;
				LOGMSG("Parse tcphdr ,buffer is invalid");
				return NULL;
			}

			m_state = 0;
			char * pTmp = pBuf;
			m_pTcpHdr = (struct tcphdr*) (pTmp);

			//pTmp +=  sizeof(struct tTcp);
			pTmp   +=  m_pTcpHdr->doff * 4;

			m_bufferLen -=m_pTcpHdr->doff*4;
			//LOGMSG("Parse tcphdr ok");
			return (char*)pTmp;
		}

		/**
		 *
		 * 解析udp头部
		 */
		char* ParseUdpHdr(char * pBuf)
		{
			if( NULL == pBuf || m_bufferLen <(sizeof(char*) * 2 ))
			{
				m_state = -4;
				LOGMSG("Parse udphdr ,buffer is invalid");
				return NULL;
			}
			m_state = 0;
			char * pTmp = pBuf;
			m_pUdpHdr = (struct udphdr*) (pTmp);
			m_bufferLen -= sizeof(udphdr);

			//LOGMSG("Parse udphdr ok");
			return (char*)(pTmp+ sizeof(udphdr));
		}

		void DumpHeadInfo()
		{
			if(NULL == m_pIpHdr ||  m_state != 0)
			{
				fprintf(stderr,"parse header error:%d\n",m_state);
				return;
			}
            else if(m_isValidPackage== false)
            {
                fprintf(stdout,"ignore packet!!!\n")
                return;
            
            }
			char szSourceIp[32] = {0};
			char szDstIp[32] = {0};
			strcpy(szSourceIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->saddr)));
			strcpy(szDstIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->daddr)));

			switch(m_pIpHdr->protocol)
			{
				case IPPROTO_TCP:
					{
						char szMsg[1024] = {0};

						LOGMSG("HEADER:[[");
						int len = snprintf(szMsg,1024,"\ttcp pkt: FROM:[%s:%u], TO:[%s:%u] \n\tttl:%u seq:%u ack_seq:%u \n"
								"\tres1:%u doff:%u fin:%u syn:%u\n\trst:%u psh:%u ack:%u urg:%u res2:%u \n\twindow:%u check:%u urg_ptr:%u\n",
								szSourceIp,ntohs(m_pTcpHdr->source),szDstIp,ntohs(m_pTcpHdr->dest),
								m_pIpHdr->ttl,m_pTcpHdr->seq,m_pTcpHdr->ack_seq,
								m_pTcpHdr->res1,m_pTcpHdr->doff,m_pTcpHdr->fin,m_pTcpHdr->syn,
								m_pTcpHdr->rst,m_pTcpHdr->psh,m_pTcpHdr->urg,m_pTcpHdr->res2,
								m_pTcpHdr->window,m_pTcpHdr->check,m_pTcpHdr->urg_ptr);
						szMsg[len >= 1024? 1024-1:len+1] = '\0';
						LOGMSG(szMsg);
						LOGMSG("]]");
						break;
					}
				case IPPROTO_UDP:
					{
						char szMsg[256]={0};
						int len = snprintf(szMsg,sizeof(szMsg),"udp pkt FROM:[%s:%u] TO:[%s:%u] ttl:%u len:%u",
								szSourceIp,ntohs(m_pUdpHdr->source),szDstIp,ntohs(m_pUdpHdr->dest),m_pIpHdr->ttl,m_pUdpHdr->len
								);
						break;
					}
				case IPPROTO_ICMP:
                { 
                    break;
                }
				case IPPROTO_IGMP:
				case IPPROTO_RAW:
					break;
				default:
					LOGMSG("unknow prototcol");
					break;
			}
		}

        int Filter()
        {
            char szSourceIp[32] = {0};
            char szDstIp[32] = {0};
            strcpy(szSourceIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->saddr)));
            strcpy(szDstIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->daddr)));

            m_isValidPackage=false;
            if(m_filter.GetSrcIp().size()!= 0 
                 && m_filter.GetSrcIp() != szSourceIp)
                return -1;
            if(m_filter.GetDstIp().size()!= 0 
                 && m_filter.GetDstIp() != szDstIp)
                return -1;

            switch(m_msgType)
            {
                case 0:
                    {
                        if(m_filter.GetDstPort() != 0  
                         && m_pTcpHdr && m_filter.GetDstPort() != m_pTcpHdr->dest)
                            return -1;

                        if(m_filter.GetSrcPort() != 0  
                         && m_pTcpHdr && m_filter.GetSrcPort() != m_pTcpHdr->source)
                            return -1;
                        break;
                    }
                case 1:
                    { 
                        if(m_filter.GetDstPort() != 0  
                         && m_pUdpHdr && m_filter.GetDstPort() != m_pUdpHdr->dest)
                            return -1;

                        if(m_filter.GetSrcPort() != 0  
                         && m_pUdpHdr && m_filter.GetSrcPort() != m_pUdpHdr->source)
                            return -1;
                        m_isValidPackage = false;
                        break;
                    }
                default:
                    m_isValidPackage = false;
            }
        }
	protected:
        //raw msg receive from ether
        char m_rawBuffer[1560];
        // data
        char m_buffer[1024*10];
        int    m_bufferLen;
        int    m_rawBufferLen;

        //protocol headers
        struct ether_header * m_pEtherHdr;
        struct  iphdr       * m_pIpHdr;
        struct  tcphdr      * m_pTcpHdr;
        struct  udphdr      * m_pUdpHdr;

        bool                  m_state;
        CFilter               m_filter;
        bool                  m_isValidPackage;
        int                   m_msgType;//0-tcp,1-udp,2-icmp
        Stat                  m_stat;
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




