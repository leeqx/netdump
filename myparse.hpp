#pragma once
#include "net.hpp"

class CMyParse: public CNetPacketParse
{
public:
	CMyParse(char* buffer,int len,CFilter filter):CNetPacketParse(buffer,len,filter)
	{
	}
	// 仅仅值分析数据
    virtual int32_t Parse()
	{
        if(this->m_state!=0)
            return -1;
        else if(this->m_isValidPackage==false)
            return -2;


        //filter packet
        if( -1 == this->Filter())
            LOGMSG("Ignore");
        
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
