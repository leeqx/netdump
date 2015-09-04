#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
using namespace std;

#define LOGINNER(msg) cout << __FILE__ <<":"<<__LINE__<<"-"<<msg<<strerror(errno)<<endl;
#define LOGMSG(msg) cout << __FILE__ <<":"<<__LINE__<<"-"<<msg<<endl;

typedef const string  CString;

class CNetModelInterface
{
	public:
		CNetModelInterface(int32_t fd):m_listendFd(fd)
		{
		}

	public:
		virtual int32_t Init()=0;
		virtual int32_t NetHandleEvent()=0;
		virtual CString & NetRecv()=0;
		virtual int32_t NetSend(CString& data)=0;

	private:
		int32_t m_listendFd;		
};

// model should be select poll epoll
template<typename model>
class CNet
{
	CNet():m_listenFd(0)
	{
	}
	virtual ~CNet()
	{
		if(m_listenFd > 0)
			close(m_listenFd);
		m_listenFd=0;
		if(m_netModel != NULL)
		{
			delete m_netModel;
			m_netModel = NULL;
		}
	}
	virtual int32_t InitNet(bool isLingger=false,bool isUnBlock=false)
	{
		m_listenFd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP|IPPROTO_UDP|IPPROTO_ICMP);
		if(m_listenFd == -1)
		{
			LOGINNER("ERROR:Create socket failed:");
			exit(-1);
		}
		bool bReuseaddr = true;
		if(setsockopt(m_listenFd,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bool)) == -1)
		{
				LOGINNER("ERROR:Set socket address reuse failed");
				exit(-1);
		}
		if(isUnBlock)
		{
			int ret = fcntl(m_listenFd,F_SETFL,O_NONBLOCK);
			if (ret == -1)
			{
				LOGINNER("ERROR:Set socket nonblock failed");
				exit(-1);
			}
		}
		
		if(isLingger)
		{
			struct linger ling={0,5};
			if(setsockopt(m_listenFd,SO_LINGER,(const char*)&ling,sizeof(bool)))
			{
				LOGINNER("ERROR:Set socket linger failed");
				exit(-1);
			}
		}

		m_netModel = new model(m_listenFd);
		if(m_netModel == NULL)
		{
				LOGINNER("ERROR:Create net model failed");
				exit(-1);
		}
		m_netModel->Init();
		m_netModel->NetHandleEvent();
	}

	private:
		int32_t m_listenFd;
		model  *m_netModel;
};




