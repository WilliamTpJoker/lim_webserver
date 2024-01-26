#include "coroutine.h"
#include "net.h"
#include "splog.h"

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_ROOT();

void run()
{
    IPAddress::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8050");
    Socket::ptr sock = Socket::CreateUDP(addr);
    if (sock->bind(addr))
    {
        LOG_INFO(g_logger) << "udp bind : " << addr->getAddr();
    }
    else
    {
        LOG_ERROR(g_logger) << "udp bind : " << addr->getAddr() << " fail";
        return;
    }
    while (true)
    {
        char buff[1024];
        Address::ptr from(new IPv4Address);
        int len = sock->recvFrom(buff, 1024, from);
        if (len > 0)
        {
            buff[len] = '\0';
            LOG_INFO(g_logger) << "recv: " << buff << " from: " << addr->getAddr();
            len = sock->sendTo(buff, len, from);
            if (len < 0)
            {
                LOG_INFO(g_logger) << "send: " << buff << " to: " << addr->getAddr() << " error=" << len;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    co_sched->start();
    co run;
    g_net->run();
    return 0;
}