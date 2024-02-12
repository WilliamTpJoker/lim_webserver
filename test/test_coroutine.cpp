#include "coroutine.h"
#include "splog.h"

#include <iostream>
#include <unistd.h>

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test_co");

static Scheduler *g_scheduler = Scheduler::Create();

static int s_count = 10000;
void run_in_co() { LOG_INFO(g_logger) << "Hello world, s_count = " << --s_count; }

void test_schduler()
{
    for (int i = 0; i < 1000; ++i)
    {
        g_scheduler->createTask(&run_in_co);
    }
    g_scheduler->start();
}

void hold(unsigned int seconds)
{
    Task *task = lim_webserver::Processor::GetCurrentTask();
    lim_webserver::TimerManager::GetInstance()->addTimer(seconds * 1000, [task] { task->wake(); });
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

    g_scheduler->createTask(&run_in_co_hold);
    g_scheduler->start();
}

int main()
{
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    g_logger->addAppender(appender);

    // test_schduler();
    test_hold();
    return 0;
}