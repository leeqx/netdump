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
		LOGMSG("now handler data\n");
		LOGMSG(m_buffer);
		return 0;
	}
};
