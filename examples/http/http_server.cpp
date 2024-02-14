#include "coroutine.h"
#include "net.h"
#include "net/http/HttpServer.h"
#include "splog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void run()
{
    Scheduler *work_sched = Scheduler::CreateNetScheduler();
    work_sched->setName("worker");
    work_sched->startInNewThread();
    http::HttpServer *server = new http::HttpServer(true, work_sched);
    server->setName("http");
    Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    server->start();
}

void add_asyncLogger()
{
    AsyncAppender::ptr aysnc = AppenderFactory::GetInstance()->defaultAsyncAppender();
    auto fileapd = AppenderFactory::GetInstance()->defaultFileAppender();
    fileapd->setFile("./log/http_log.txt");
    aysnc->bindAppender(fileapd);
    LOG_SYS()->addAppender(aysnc);
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_TRACE;

    if (argc == 2)
    {
        level = LogLevel_DEBUG;
    }
    LOG_SYS()->setLevel(level);
    LOG_SYS()->detachAppender("console");
    add_asyncLogger();

    Scheduler *accept_sched = Scheduler::CreateNetScheduler();
    

    accept_sched->setName("accepter");
    accept_sched->createTask(&run);
    accept_sched->start();
    return 0;
}