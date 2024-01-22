#include "coroutine/coroutine.h"
#include "splog/splog.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test_co");

static int s_count = 10000;
void run_in_co()
{
    LOG_INFO(g_logger) << "Hello world, s_count = " << --s_count;
}

void test_schduler()
{
    g_Scheduler->start(2);
    for (int i = 0; i < 10000; ++i)
    {
        g_Scheduler->createTask(&run_in_co);
    }
    sleep(1);
}



void hold(unsigned int seconds)
{
    uint64_t id = lim_webserver::Processor::GetCurrentTask()->id();
    lim_webserver::Processor *processor = lim_webserver::Processor::GetCurrentProcessor();
    lim_webserver::TimerManager::GetInstance()->addTimer(seconds * 1000,
                                                         [processor, id]
                                                         {
                                                             processor->wakeupTask(id);
                                                         });
    lim_webserver::Processor::CoHold();
}

void run_in_co_hold()
{
    LOG_INFO(g_logger) << "Hello world";
    LOG_INFO(g_logger) << "before hold";
    hold(2);
    LOG_INFO(g_logger) << "after hold";
}

void test_hold()
{
    g_Scheduler->start(1);

    g_Scheduler->createTask(&run_in_co_hold);
    sleep(3);
}

int main()
{
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    g_logger->addAppender(appender);
    
    // test_schduler();
    test_hold();
    
    return 0;
}