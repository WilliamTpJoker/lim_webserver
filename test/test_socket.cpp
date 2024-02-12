#include "coroutine.h"
#include "net.h"
#include "splog.h"

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_ROOT();

static Scheduler *g_net = Scheduler::CreateNetScheduler();

void test_socket()
{
    IPAddress::ptr addr = Address::LookupAnyIPAddress("www.baidu.com");
    if (addr)
    {
        LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    Socket::ptr sock = Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr))
    {
        LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail.";
        return;
    }
    else
    {
        LOG_INFO(g_logger) << "connnect " << addr->toString() << " success.";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0)
    {
        LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(2048);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt <= 0)
    {
        LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }
    buffs.resize(rt);
    LOG_INFO(g_logger) << buffs;
}

void stop_sched()
{
    sleep(3);
    g_net->stop();
}

int main(int argc, char *argv[])
{
    LogLevel level = LogLevel_TRACE;
    if (argc == 2)
    {
        level = LogLevel_DEBUG;
    }
    LOG_SYS()->setLevel(level);

    g_net->createTask(&test_socket);
    g_net->createTask(&stop_sched);
    g_net->start();
    return 0;
}