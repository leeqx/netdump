#pragma once
#include "net.hpp"
#include <sys/select.h>
#include <vector>


template<typename Parse>
class CSelect : public CNetModelInterface
{
	public:
		CSelect(int32_t sockfd,string host,int32_t port):CNetModelInterface(sockfd,host,port),m_clientAddrLen(sizeof(m_clientAddr))
		{
		}
	public:
		int32_t Init()
		{
			FD_ZERO(&m_readSet);
			FD_ZERO(&m_writeSet);

			FD_SET(m_netSockFd,&m_readSet);
			FD_SET(m_netSockFd,&m_writeSet);
			m_events.clear();
			m_events.push_back(m_netSockFd);
			return 0;
		}
		int32_t NetHandleEvent()
		{
			fprintf(stdout,"%s\n",__func__);
			NetRecv();
			return 0;
		}
		int32_t NetRecv()
		{
			fprintf(stdout,"%s\n",__func__);
			int ret = 0;
			int maxfd=m_netSockFd+1;
			char buffer[1560] = {0};
			
			while(1)
			{
				FD_ZERO(&m_readSet);
				FD_ZERO(&m_writeSet);

				FD_SET(m_netSockFd,&m_readSet);
				FD_SET(m_netSockFd,&m_writeSet);

				ret = select(maxfd	,&m_readSet,&m_writeSet,NULL,0);
				switch(ret)
				{
					case -1:
					{
						LOGINNER("ERROR:select error");
						break;
					}
					case 0:
					{
						LOGINNER("select timeout,try again");
						break;
					}
					default:
					{
						//read event
						if(FD_ISSET(m_netSockFd,&m_readSet))
						{
							if(FD_ISSET(m_netSockFd,&m_readSet))
							{
								memset(buffer,0x00,sizeof(buffer));
								int len = recvfrom(m_netSockFd,(char*)buffer,sizeof(buffer),0,(struct sockaddr*)&m_clientAddr,(socklen_t*)&m_clientAddrLen);
								if(len == -1)
								{
									LOGINNER("recv return -1");
									continue;
								}
								else if(len == 0)
								{
									continue;
								}
								Parse parser(buffer,len);
								parser.Parse();
							}
						}

						// write event
						if(FD_ISSET(m_netSockFd, &m_writeSet))
						{
						}
					}//default
				}//switch
			}
			return 0;
		}
		int32_t NetSend(CString& data)
		{
			return 0;
		}
	private:
		fd_set m_readSet;
		fd_set m_writeSet;
		struct sockaddr_in m_clientAddr;
		socklen_t   m_clientAddrLen;
		vector<int32_t> m_events;
};

