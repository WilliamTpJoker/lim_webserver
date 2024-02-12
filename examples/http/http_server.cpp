#include "coroutine.h"
#include "net.h"
#include "net/http/HttpServer.h"
#include "splog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_ROOT();

static Scheduler *g_net = Scheduler::CreateNetScheduler();

void run()
{
    http::HttpServer *server = new http::HttpServer(true);
    server->setName("http");
    Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    server->start();
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_TRACE;
    lim_webserver::AsyncAppender::ptr aysnc = AppenderFactory::GetInstance()->defaultAsyncAppender();
    auto fileapd = AppenderFactory::GetInstance()->defaultFileAppender();
    fileapd->setFile("./log/http_log.txt");
    aysnc->bindAppender(fileapd);
    if (argc == 2)
    {
        level = LogLevel_DEBUG;
    }
    LOG_SYS()->setLevel(level);
    LOG_SYS()->addAppender(aysnc);
    LOG_SYS()->detachAppender("console");

    g_net->createTask(&run);
    g_net->start();
    return 0;
}