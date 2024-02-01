#include "coroutine.h"
#include "splog.h"

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
    g_Scheduler->start(1);
    for (int i = 0; i < 10000; ++i)
    {
        fiber run_in_co;
    }
    sleep(1);
}



void hold(unsigned int seconds)
{
    Task* task = lim_webserver::Processor::GetCurrentTask();
    lim_webserver::TimerManager::GetInstance()->addTimer(seconds * 1000,
                                                         [task]
                                                         {
                                                            task->wake();
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
    g_Scheduler->start();

    fiber run_in_co_hold;
    sleep(3);
}

int main()
{
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    g_logger->addAppender(appender);
    
    test_schduler();
    // test_hold();
    return 0;
}