#include "splog.h"
#include "net/Address.h"

lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void test()
{
    std::vector<lim_webserver::Address::ptr> addrs;

    bool v = lim_webserver::Address::Lookup(addrs, "www.baidu.com:http");
    if(!v)
    {
        LOG_ERROR(g_logger)<<"lookup failed";
    }
    for(int i=0;i<addrs.size();++i)
    {
        LOG_INFO(g_logger)<<addrs[i]->toString();
    }
}

void testIpv4()
{
    auto ip = lim_webserver::IPAddress::Create("180.101.50.188",80);
    if(ip)
    {
        LOG_INFO(g_logger)<<ip->toString();
    }
    
}

int main(int argc, char *args[])
{
    // test();
    testIpv4();

    return 0;
}