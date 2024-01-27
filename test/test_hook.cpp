#include "splog.h"
#include "coroutine.h"
#include "net/EventLoop.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

lim_webserver::Logger::ptr g_logger = LOG_ROOT();

// 通过hook将同步的事件转换为异步的事件
void test_sleep()
{
    LOG_INFO(g_logger) << "test_sleep: start";
    co[]
    {
        sleep(2);
        LOG_INFO(g_logger) << "test_sleep: 2 seconds";
    };

    co[]
    {
        sleep(3);
        LOG_INFO(g_logger) << "test_sleep: 3 seconds";
    };
}

void test_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "180.101.50.242", &addr.sin_addr.s_addr);

    LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    LOG_INFO(g_logger) << "connect rt=" << rt << " errno = " << errno << " strerror = " << strerror(errno);

    if (rt)
    {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LOG_INFO(g_logger) << "send rt=" << rt << " errno = " << errno << " strerror = " << strerror(errno);

    if (rt <= 0)
    {
        return;
    }

    std::string buff;
    buff.resize(2048);

    rt = recv(sock, &buff[0], buff.size(), 0);
    LOG_INFO(g_logger) << "recv rt=" << rt << " errno = " << errno << " strerror = " << strerror(errno);

    if (rt <= 0)
    {
        return;
    }

    buff.resize(rt);
    LOG_INFO(g_logger) << buff;
}

int main(int argc, char *args[])
{
    lim_webserver::LogLevel level = LogLevel_DEBUG;
    if (argc == 2)
    {
        level = LogLevel_TRACE;
    }
    LOG_SYS()->setLevel(level);
    co_sched->start();
    // co test_sleep;
    // sleep(4);
    co test_socket;
    lim_webserver::EventLoop::GetInstance()->run();
    return 0;
}