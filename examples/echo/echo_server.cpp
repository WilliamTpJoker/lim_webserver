#include "coroutine.h"
#include "net.h"
#include "splog.h"

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_ROOT();

class EchoServer : public TcpServer
{
public:
    EchoServer(const std::string &name) : TcpServer(name) {}

    void handleClient(Socket::ptr client) override;
};

void EchoServer::handleClient(Socket::ptr client)
{
    LOG_INFO(g_logger) << "handleClient " << client->peerAddress()->getAddr();
    ByteArray::ptr ba(new ByteArray);
    while (true)
    {
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0)
        {
            LOG_INFO(g_logger) << "client close: " << client->fd();
            break;
        }
        else if (rt < 0)
        {
            LOG_INFO(g_logger) << "client error rt=" << rt
                               << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);

        std::cout << ba->toString();
        std::cout.flush();
    }
}

void run()
{
    LOG_INFO(g_logger) << "server start";
    EchoServer *es = new EchoServer("echo");
    auto addr = Address::LookupAny("0.0.0.0:8020");
    while (!es->bind(addr))
    {
        sleep(2);
    }
    es->start();
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_TRACE;
    lim_webserver::AsyncAppender::ptr aysnc = AppenderFactory::GetInstance()->defaultAsyncAppender();
    aysnc->bindAppender(AppenderFactory::GetInstance()->defaultFileAppender());
    if (argc == 2)
    {
        level = LogLevel_DEBUG;
    }
    LOG_SYS()->setLevel(level);
    LOG_SYS()->addAppender(aysnc);

    co_sched->start();
    co run;
    g_net->run();
    return 0;
}
