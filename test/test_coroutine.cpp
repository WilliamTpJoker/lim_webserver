#include "Scheduler.h"
#include "SpikeLog.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test_co");

static int s_count = 10000;
void run_in_co()
{
    LOG_INFO(g_logger)<<"Hello world, s_count = "<<--s_count;
}

void test_schduler()
{
    g_Scheduler->start(2);
    for(int i=0;i<10000;++i)
    {
        g_Scheduler->createTask(&run_in_co);
    }
    sleep(1);
}

int main()
{   
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    appender->setFormatter("%d%T%t %N%T%C%T[%c] [%p] %f:%l%T%m%n");
    g_logger->addAppender(appender);
    test_schduler();
    return 0;
}