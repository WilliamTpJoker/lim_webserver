#include "coroutine.h"
#include "net.h"
#include "net/http/HttpServer.h"
#include "splog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void run() 
{
    http::HttpServer* server = new http::HttpServer("http");
    Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

int main(int argc, char *argv[])
{
    lim_webserver::LogLevel level = LogLevel_INFO;
    LOG_SYS()->setLevel(level);

    fiber_sched->start();
    fiber run;
    g_net->idle();
    return 0;
}
