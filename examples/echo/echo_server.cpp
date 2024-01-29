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
    LOG_INFO(g_logger) << "handleClient " << client->peerAddress()->toString();
    ByteArray::ptr buf = ByteArray::Create();
    while (true)
    {
        int rt = client->recv(buf, 1024);
        if (rt == 0)
        {
            LOG_INFO(g_logger) << "client close: " << client->peerAddress()->toString();
            break;
        }
        else if (rt < 0)
        {
            LOG_INFO(g_logger) << "client error rt=" << rt
                               << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }

        std::cout << buf->toString();
        std::cout.flush();
        buf->clear();
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
    // LOG_SYS()->addAppender(aysnc);

    co_sched->start();
    co run;
    g_net->run();
    return 0;
}
