#include "lim.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

lim_webserver::Logger::ptr g_logger = LIM_LOG_ROOT();

int sock;

void test_fiber()
{
    LIM_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "180.101.50.188", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr *)&addr, sizeof(addr)))
    {
        LIM_LOG_ERROR(g_logger)<< "connect error";
    }
    else if (errno == EINPROGRESS)
    {
        lim_webserver::IoManager::GetThis()->addEvent(sock, lim_webserver::IoManager::READ, []()
                                                      { LIM_LOG_INFO(g_logger) << "read callback"; });
        lim_webserver::IoManager::GetThis()->addEvent(sock, lim_webserver::IoManager::WRITE, []()
                                                      { LIM_LOG_INFO(g_logger) << "write callback";
                                                      lim_webserver::IoManager::GetThis()->cancelEvent(sock, lim_webserver::IoManager::READ);
                                                        close(sock); 
                                                        });
        
    }
    else
    {
        LIM_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test()
{
    lim_webserver::IoManager iom(1, true,"test");
    iom.schedule(&test_fiber);
}

int main(int argc, char *argv[])
{
    test();
    return 0;
}