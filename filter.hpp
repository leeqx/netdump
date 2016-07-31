/**
 * 用于过滤指定的数据包例如端口、ip、监听包的数量
 *
 *
 */
#pragma once

#include <string>
using std::string;

#define BOTH 0x03
#define DEST 0x01
#define SOURCE 0x02

class CFilter
{
    public:
        CFilter():
            m_strSrcIp(""),
            m_strDstIp(""),
            m_strEtherType(""),
            m_srcPort(0),
            m_dstPort(0),
            m_packetCount(0),
            m_packetSize(0),
            m_protoType(0),
            m_bothPort(0)
        {
        }

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
            m_portType = SOURCE;
        }
        void SetDstPort(int port)
        {
            m_dstPort = port;
            m_portType = DEST;
        }

        void SetBothPort(int port)
        {
            m_bothPort = port;
            m_portType = BOTH;
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

        void SetEtherType(string ether)
        {
            m_strEtherType = ether;
        }

        const string& GetSrcIp()
        {
            return    m_strSrcIp;
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
        int GetBothPort()
        {
            return m_bothPort;
        }

        int GetPacketCount()
        {
            return m_packetCount  ;
        }
        int GetProtoType()
        {
            return m_protoType;
        }
        const string& GetEtherType()
        {
            return m_strEtherType;
        }
        int GetPortType()
        {
            return m_portType;
        }
    protected:
        string m_strSrcIp;
        string m_strDstIp;
        string m_strEtherType;
        int    m_srcPort;
        int    m_dstPort;
        int    m_bothPort;
        int    m_portType;

        int    m_packetCount;
        int    m_packetSize;
        int    m_protoType;
};


