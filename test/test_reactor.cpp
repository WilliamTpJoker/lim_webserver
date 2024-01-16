#include "EventLoop.h"
#include "SpikeLog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test");

int sock;

void test_reactor()
{
    EventLoop* eventloop = new EventLoop();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "180.101.50.188", &addr.sin_addr.s_addr);
    if (!connect(sock, (const sockaddr *)&addr, sizeof(addr)))
    {
        LOG_ERROR(g_logger)<< "connect error";
    }
    else if (errno == EINPROGRESS)
    {
        LOG_INFO(g_logger)<<"!";
        
    }

    eventloop->run();
}

int main()
{
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    appender->setFormatter("%d%T%t %N%T[%c] [%p] %f:%l%T%m%n");
    g_logger->addAppender(appender);

    test_reactor();
    
    return 0;
}