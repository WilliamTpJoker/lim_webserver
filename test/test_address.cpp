#include "address.h"
#include "log.h"

lim_webserver::Logger::ptr g_logger = LIM_LOG_ROOT();

void test()
{
    std::vector<lim_webserver::Address::ptr> addrs;

    bool v = lim_webserver::Address::Lookup(addrs, "www.baidu.com:http");
    if(!v)
    {
        LIM_LOG_ERROR(g_logger)<<"lookup failed";
    }
    for(int i=0;i<addrs.size();++i)
    {
        LIM_LOG_INFO(g_logger)<<addrs[i]->toString();
    }
}

void testIpv4()
{
    auto ip = lim_webserver::IPAddress::Create("180.101.50.188",80);
    if(ip)
    {
        LIM_LOG_INFO(g_logger)<<ip->toString();
    }
    
}

int main(int argc, char *args[])
{
    // test();
    testIpv4();

    return 0;
}