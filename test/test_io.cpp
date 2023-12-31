#include "SpikeLog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

lim_webserver::Logger::ptr g_logger = LOG_ROOT();

int sock;

void test_fiber()
{
    LOG_INFO(g_logger) << "test_fiber sock=" << sock;

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
        lim_webserver::IoManager::GetThis()->addEvent(sock, lim_webserver::IoManager::READ, []()
                                                      { LOG_INFO(g_logger) << "read callback"; });
        lim_webserver::IoManager::GetThis()->addEvent(sock, lim_webserver::IoManager::WRITE, []()
                                                      { LOG_INFO(g_logger) << "write callback";
                                                      lim_webserver::IoManager::GetThis()->cancelEvent(sock, lim_webserver::IoManager::READ);
                                                        close(sock); 
                                                        });
        
    }
    else
    {
        LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test()
{
    lim_webserver::IoManager iom(1, true,"test");
    iom.schedule(&test_fiber);
}

lim_webserver::Timer::ptr s_timer;
void test_timer()
{
    lim_webserver::IoManager iom(2);
    s_timer =iom.addTimer(500,[](){
        static int i=0;
        LOG_INFO(g_logger)<<"hello timer, i="<<i;
        if(++i==3)
        {
            s_timer->reset(2000);
            // s_timer->cancel();
        }
    },true);
    // iom.schedule(&test_fiber);
}

int main(int argc, char *argv[])
{
    // test();
    test_timer();
    return 0;
}