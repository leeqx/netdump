#pragma once
#include <string.h>
#include <vector>
#include <algorithm>

#include "net.hpp"
#include "netparse.h"

using std::string;
using std::map;
using std::pair;

typedef struct _Value
{
    string key;
    string value;
    _Value(string k,string v):key(k),value(v)
    {
    }
}Value;

struct redisPackPrint{
    void operator ()(Value data)
    {
        fprintf(stdout,"%s=%s\n",data.key.c_str(),data.value.c_str()); 
    }
};

typedef struct redispack{
    char nNumber;
    vector<Value> kvs;
}RedisPackage;

class CRedisParse: public CNetPacketParse
{
    public:
        CRedisParse(char* m_buffer,int len,CFilter filter):CNetPacketParse(m_buffer,len,filter)
        {
        }
        /**
         * 请求包:
         *    * \r\n  $ \r\n .. $ \r\n \r\n
         *    eg: *3 $3 SET $5 mykey $7 myvalue
         *     "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n\r\n
         * 应答包:
         *   如果是单行回复，那么第一个字节是「+」
         *   如果回复的内容是错误信息，那么第一个字节是「-」
         *   如果回复的内容是一个整型数字，那么第一个字节是「:」
         *   如果是bulk回复，那么第一个字节是「$」
         *   如果是multi-bulk回复，那么第一个字节是「*」
         */
        virtual int32_t Parse()
        {
            if(this->m_state != 0)
                return -1;
            else if(this->m_isValidPackage == false)
                return -2;

            if(this->GetBufferLen() >0)
            {
                if(m_buffer)
                {
                    switch((char)m_buffer[0])
                    {
                        case ':':
                            {
                                fprintf(stdout,"value=[%s]\n",m_buffer+1);
                                break;
                            }
                        case '+':
                            {
                                fprintf(stdout,"value=[%s]\n",m_buffer+1);
                                break;
                            }
                        case '-':
                            {
                                fprintf(stdout,"value=[%s]\n",m_buffer+1);
                                break;
                            }
                        case '$':
                            {
                                int i = 0;
                                for(char*pos = strtok(m_buffer+1,"\r\n");pos;pos=strtok(NULL,"\r\n"),i++)
                                {
                                    if(i % 2 == 0)
                                    {
                                        fprintf(stdout,"len=[%s]\n",pos);
                                    }
                                    else
                                    {
                                        fprintf(stdout,"value=[%s]\n",pos);
                                    }
                                }
                                break;
                            }
                        case '*':
                            {
                                RedisPackage redisVal;
                                int i = 0;
                                for(char *pos = strtok(m_buffer+1,"\r\n");pos;pos=strtok(NULL,"\r\n"),i++)
                                {
                                    if( i % 2 == 0 && i == 0)
                                    {
                                        redisVal.kvs.push_back(Value("total",(char*)pos));
                                    }
                                    else if (i % 2 == 1)
                                    {
                                        redisVal.kvs.push_back(Value("len",(char*)pos));
                                    }
                                    else
                                    {
                                        redisVal.kvs.push_back(Value("value",(char*)pos));
                                    }

                                }
                                for_each(redisVal.kvs.begin(),redisVal.kvs.end(),redisPackPrint());
                                break;
                            }
                        default:
                            LOGMSG("unexpected m_buffer");
                    }
                }
                else
                {
                   // LOGMSG("error: memory operator failed:");
                }
            }
        }
};

