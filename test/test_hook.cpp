#include "hook.h"
#include "io_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

lim_webserver::Logger::ptr g_logger = LIM_LOG_ROOT();

// 通过hook将同步的事件转换为异步的事件
void test_sleep()
{
    lim_webserver::IoManager iom(1);
    iom.schedule([]()
                 {
        sleep(2);
        LIM_LOG_INFO(g_logger)<<"sleep 2"; });
    iom.schedule([]()
                 {
        sleep(3);
        LIM_LOG_INFO(g_logger)<<"sleep 3"; });
    LIM_LOG_INFO(g_logger) << "test sleep";
}

void test_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "180.101.50.188", &addr.sin_addr.s_addr);

    LIM_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    LIM_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if (rt)
    {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LIM_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    LIM_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    buff.resize(rt);
    LIM_LOG_INFO(g_logger) << buff;
}

int main(int argc, char *args[])
{
    // test_sleep();
    lim_webserver::IoManager iom;
    iom.schedule(test_socket);

    return 0;
}