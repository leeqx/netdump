#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <algorithm>

using std::map;
using std::string;


struct DumpStat
{
    void operator ()(map<string,int>::iterator it)
    {
        fprintf(stdout,"%s=%d\n",it->first.c_str(),it->second)
    }
};
struct Stat
{
    map<string,int> m_netstat;
    void dump()
    {
        for_each(m_netstat.begin(),m_netstat.end(),DumpStat);
    }
};

