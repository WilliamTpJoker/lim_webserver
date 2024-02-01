#include "coroutine.h"
#include "net.h"
#include "net/http/HttpServer.h"
#include "splog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void run() 
{
    http::HttpServer* server = new http::HttpServer("http",true);
    Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_TRACE;
    lim_webserver::AsyncAppender::ptr aysnc = AppenderFactory::GetInstance()->defaultAsyncAppender();
    auto fileapd = AppenderFactory::GetInstance()->defaultFileAppender();
    fileapd->setFile("http_log.txt");
    aysnc->bindAppender(fileapd);
    if (argc == 2)
    {
        level = LogLevel_DEBUG;
    }
    LOG_SYS()->setLevel(level);
    LOG_SYS()->addAppender(aysnc);
    LOG_SYS()->detachAppender("console");
    
    fiber_sched->start();
    fiber run;
    g_net->run();
    return 0;
}