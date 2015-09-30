#pragma once
#include "net.hpp"

class CMyParse: public CNetPacketParse
{
public:
    virtual int32_t Parse(const string &buffer)
	{
			m_buffer = buffer;
			fprintf(stdout,"%s\n",buffer.c_str());
			return buffer.size();
	}
};
