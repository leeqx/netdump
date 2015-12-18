#include "net.hpp"

#pragma once
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
                                            m_msgType(-1),
                                            m_isValidPackage(false)
                                                    
		{
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
                    m_isValidPackage = false;
					LOGMSG("parse header,unknow prototcol");
					break;
			}
			memcpy(m_buffer ,(char*)tmp,m_bufferLen);

            this->Filter();
            if(this->m_isValidPackage == true)
            {
                this->DumpHeadInfo(); 
            }
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
                m_stat.m_netstat["ether_error"]++;
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
            m_isValidPackage = true;
			return (char*)(pTmp + sizeof(struct ether_header));
		} 
		/** 
		 * 解析ip头部 
		 */ 
		char* ParseIpHdr(char* pBuf)
		{ 
			if(NULL == pBuf || m_bufferLen < 20 || m_state) {
				m_state = -2;
                m_stat.m_netstat["ip_error"]++;
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

            m_isValidPackage = true;
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
                m_stat.m_netstat["tcp_error"]++;
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
            m_isValidPackage = true;
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
                m_stat.m_netstat["udp_error"]++;
				LOGMSG("Parse udphdr ,buffer is invalid");
				return NULL;
			}
			m_state = 0;
			char * pTmp = pBuf;
			m_pUdpHdr = (struct udphdr*) (pTmp);
			m_bufferLen -= sizeof(udphdr);

			//LOGMSG("Parse udphdr ok");
            m_isValidPackage = true;
			return (char*)(pTmp+ sizeof(udphdr));
		}

		void DumpHeadInfo()
		{
			if(NULL == m_pIpHdr ||  m_state != 0) // packet parse state
			{
				fprintf(stderr,"parse header error:%d\n",m_state);
				return;
			}
            else if(m_isValidPackage== false) // filter pattern
            {
                fprintf(stdout,"ignore packet!!!\n");
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
            char szDstIp[32]    = {0};
            strcpy(szSourceIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->saddr)));
            strcpy(szDstIp,inet_ntoa(*(struct in_addr*)&(m_pIpHdr->daddr)));
 
            m_isValidPackage = false;

            if( m_filter.GetPacketCount() != 0 &&
                 m_filter.GetPacketCount() == m_stat.m_netstat["all"]
             )
            {
                m_stat.dump();
                exit(0);
            }

            if(m_filter.GetSrcIp().size()!= 0 
                 && m_filter.GetSrcIp() != szSourceIp)
                return -1;

            if(m_filter.GetDstIp().size()!= 0 
                 && m_filter.GetDstIp() != szDstIp)
                return -2;

            switch(m_msgType)
            {
                case 0:
                    {
                        if(m_filter.GetDstPort() != 0  
                         && m_pTcpHdr && m_filter.GetDstPort() != ntohs(m_pTcpHdr->dest))
                        {
                            return -3;
                        }

                        if(m_filter.GetSrcPort() != 0  
                         && m_pTcpHdr && m_filter.GetSrcPort() != ntohs(m_pTcpHdr->source))
                        {
                            return -4;
                        }
                        break;
                    }
                case 1:
                    { 
                        if(m_filter.GetDstPort() != 0  
                         && m_pUdpHdr && m_filter.GetDstPort() != ntohs(m_pUdpHdr->dest))
                        {
                            return -5;
                        }

                        if(m_filter.GetSrcPort() != 0  
                         && m_pUdpHdr && m_filter.GetSrcPort() != ntohs(m_pUdpHdr->source))
                        {
                            return -6;
                        }
                        break;
                    }
                default:
                    break;
            }
            m_isValidPackage = true;
            return 0;
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
        static Stat           m_stat;
};

Stat CNetPacketParse::m_stat={};


