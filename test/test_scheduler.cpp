#include "SpikeLog.h"

using namespace lim_webserver;

Logger::ptr g_logger = LOG_NAME("test");

static int s_count1 = 500;

static int s_count2 = 5;
void run_in_fiber()
{
    LOG_INFO(g_logger) << "test in fiber, s_count=" << --s_count1;
}

void run_in_fiber2()
{
    LOG_INFO(g_logger) << "test in fiber, s_count=" << s_count2;
    // lim_webserver::Fiber::YieldToReady();
    sleep(1);
    if (--s_count2 >= 0)
    {
        lim_webserver::Sched::GetThis()->schedule(&run_in_fiber2);
    }
}

void test(int threads, bool use_caller, std::string name)
{
    std::vector<std::function<void()>> f;
    for (int i = 1; i < 500; ++i)
    {
        f.push_back(&run_in_fiber);
    }
    LOG_INFO(g_logger) << " main";
    lim_webserver::Sched sc(threads, use_caller, name);
    sc.start();
    LOG_INFO(g_logger) << " schedule";
    sc.schedule(f.begin(), f.end());
    sc.stop();
    LOG_INFO(g_logger) << " over" << s_count1;
}

void test2(int threads, bool use_caller, std::string name)
{

    LOG_INFO(g_logger) << " main";
    lim_webserver::Sched sc(threads, use_caller, name);
    sc.start();
    LOG_INFO(g_logger) << " schedule";
    sc.schedule(&run_in_fiber2);
    sc.stop();
    LOG_INFO(g_logger) << " over" << s_count2;
}

int main(int argc, char *argv[])
{
    ConsoleAppender::ptr appender =AppenderFcty::GetInstance()->defaultConsoleAppender();
    g_logger->addAppender(appender);
    // g_l->setLevel(LogLevel_INFO);
    // test(3, true, "test");
    test(2, false, "test");
    return 0;
}