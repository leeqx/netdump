#pragma once
#include "net.hpp"

class CMyParse: public CNetPacketParse
{
public:
	CMyParse(char* buffer,int len):CNetPacketParse(buffer,len)
	{
	}
	// 仅仅值分析数据
    virtual int32_t Parse()
	{
		if (this->GetBufferLen() > 0)
		{
		LOGMSG("BODY:[[");
		LOGMSG(m_buffer);
		LOGMSG("]]");
		}
		else
		{
			LOGMSG("warn:BODY is empty !!!!");
		}
		return 0;
	}
};
