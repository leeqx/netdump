#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <algorithm>

using std::map;
using std::string;


struct DumpStat
{
    void operator ()(map<string,int>::value_type it)
    {
        fprintf(stdout,"%10s packet=%d\n",it.first.c_str(),it.second);
    }
};
struct Stat
{
    map<string,int> m_netstat;
    void dump()
    {
        fprintf(stdout,"===============packet statistic============\n");
        for_each(m_netstat.begin(),m_netstat.end(),DumpStat());
        fprintf(stdout,"===============packet statistic============\n");
    }
};

