#include "splog.h"
#include "coroutine.h"
#include "net.h"

static lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void test_socket()
{
    lim_webserver::IPAddress::ptr addr = lim_webserver::Address::LookupAnyIPAddress("www.baidu.com");
    if (!addr)
    {
        LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    lim_webserver::Socket::ptr sock = lim_webserver::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr))
    {
        LOG_ERROR(g_logger) << "connect " << addr->toString() << "fail";
    }
    else
    {
        LOG_INFO(g_logger) << "connnect" << addr->toString() << " success";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0)
    {
        LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt <= 0)
    {
        LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }
    buffs.resize(rt);
    LOG_INFO(g_logger) << buffs;
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_DEBUG;
    if (argc == 2)
    {
        level = LogLevel_TRACE;
    }
    LOG_SYS()->setLevel(level);

    co_sched->start();
    co test_socket;
    g_net->run();
    return 0;
}