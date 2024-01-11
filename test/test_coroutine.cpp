#include "Scheduler.h"
#include "SpikeLog.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test_co");
auto appender = AppenderFcty::GetInstance()->defaultConsoleAppender();

void run_in_co()
{
    LOG_INFO(g_logger)<<"Hello world";
}

void test_schduler()
{
    g_Scheduler->start(1);
    for(int i=0;i<10;++i)
    {
        g_Scheduler->createTask(&run_in_co);
    }
    sleep(1);
}

int main()
{   
    appender->setFormatter("%d%T%t %N%T%C%T[%c] [%p] %f:%l%T%m%n");
    g_logger->addAppender(appender);
    test_schduler();
    return 0;
}