#pragma once
#include "net.hpp"
#include <sys/select.h>
#include <vector>

const int32_t LISTEN_BACKLOG =  5;

template<typename Parse>
class CSelect : public CNetModelInterface
{
	public:
		CSelect(int32_t sockfd,string host,int32_t port):CNetModelInterface(sockfd,host,port)
		{
		}
	public:
		int32_t Init()
		{
			FD_ZERO(&m_readSet);
			FD_ZERO(&m_writeSet);

			FD_SET(m_netSockFd,&m_readSet);
			FD_SET(m_netSockFd,&m_writeSet);
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr= inet_addr(m_host.c_str());
			addr.sin_port = htons(m_port);
			if(bind(m_netSockFd,(struct sockaddr*)&addr,sizeof(addr)))
			{
				LOGINNER("ERROR:Bind error");
				exit(-1);
			}
			if(listen(m_netSockFd,LISTEN_BACKLOG))
			{
				LOGINNER("ERROR:listen error");
				exit(-1);
			}
			m_events.clear();
			m_events.push_back(m_netSockFd);
			return 0;
		}
		int32_t NetHandleEvent()
		{
			return 0;
		}
		int32_t NetRecv()
		{
			int ret = 0;
			int maxfd=m_netSockFd+1;
			char buffer[10240] = {0};
			int allfd[1024];
			
			while(1)
			{
				FD_ZERO(&m_readSet);
				FD_ZERO(&m_writeSet);

				FD_SET(m_netSockFd,&m_readSet);
				FD_SET(m_netSockFd,&m_writeSet);

				for(int i = 0;i< LISTEN_BACKLOG;i++)
				{
					if(allfd[i] > 0)
					{
						FD_SET(allfd[i],&m_readSet);
						FD_SET(allfd[i],&m_writeSet);
					}
				}
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
							if(m_type == PROTO_UDP)
							{
								memset(buffer,0x00,sizeof(buffer));
								int len = recvfrom(m_netSockFd,buffer,sizeof(buffer),0,(struct sockaddr*)&m_clientAddr,&m_clientAddrLen);
								if(len < 0)
								{
									fprintf(stderr,"read data error\n");
								}
							}
							else
							{
								if(FD_ISSET(m_netSockFd,&m_readSet))
								{
									int clientfd = accept(m_netSockFd,(struct sockaddr*)&m_clientAddr,&m_clientAddrLen);
									if(clientfd > 0)
									{
										int i = 0;
										for(i=0;i<LISTEN_BACKLOG;i++)
										{
											if(allfd[i]==-1)
											{
												allfd[i] = clientfd;
												break;
											}
										}
										if(i== LISTEN_BACKLOG)
										{
											close(clientfd);
											LOGINNER("ERROR:listen queue is full");
										}
									}
								}
							}
						}

						// read data
						for(int i = 0;i<LISTEN_BACKLOG;i++)
						{
							if(allfd[i] > 0 && FD_ISSET(allfd[i],&m_readSet))
							{
								memset(buffer,0x00,sizeof(buffer));
								int len = recv(allfd[i],buffer,sizeof(buffer),0);
								//client close
								if(len < 0)
								{
									fprintf(stderr,"client close:%d\n",allfd[i]);
									FD_CLR(allfd[i],&m_readSet);
									FD_CLR(allfd[i],&m_writeSet);
									close(allfd[i]);
									allfd[i] = -1;
								}
								else
								{
									//调用指定的解析协议进行包解析
									Parse parser;
									parser.Parse(buffer);
								}
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

