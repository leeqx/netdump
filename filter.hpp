/**
 * 用于过滤指定的数据包例如端口、ip、监听包的数量
 *
 *
 */
#pragma once

#include <string>
using std::string;

class CFilter
{
	public:
		void SetSrcIp(string srcIp)
		{
			m_strSrcIp=srcIp;
		}
		void SetDstIp(string dstIp)
		{
			m_strDstIp = dstIp;
		}
		void SetSrcPort(int port)
		{
			m_srcPort = port;
		}
		void SetDstPort(int port)
		{
			m_dstPort = port;
		}

		void SetPacketCount(int count)
		{
			m_packetCount = count;
		}
		void SetPacketSize(int size)
		{
			m_packetSize= size;
		}
		void SetProtoType(int type)
		{
			m_protoType = type;
		}

		const string& GetSrcIp()
		{
			return	m_strSrcIp;
		}
		const string& GetDstIp()
		{
			return m_strDstIp;
		}
		int GetSrcPort()
		{
			return m_srcPort ;
		}
		int GetDstPort()
		{
			return m_dstPort  ;
		}

		int GetPacketCount()
		{
			return m_packetCount  ;
		}
		int GetProtoType()
		{
			return m_protoType;
		}
	protected:
		string m_strSrcIp;
		string m_strDstIp;
		int    m_srcPort;
		int    m_dstPort;

		int    m_packetCount;
		int    m_packetSize;
		int    m_protoType;
		
	
};


