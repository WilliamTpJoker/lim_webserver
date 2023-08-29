#include "lim.h"

lim_webserver::Shared_ptr<lim_webserver::Logger> g_logger = LIM_LOG_ROOT();
static int s_count = 500;

void run_in_fiber()
{
    LIM_LOG_INFO(g_logger) << "test in fiber, s_count=" << s_count;
    if (--s_count >= 0)
    {
        lim_webserver::Scheduler::GetThis()->schedule(&run_in_fiber);
    }
}

void test(int threads, bool use_caller, std::string name)
{
    LIM_LOG_INFO(g_logger) << " main";
    lim_webserver::Scheduler sc(threads, use_caller, name);
    sc.start();
    LIM_LOG_INFO(g_logger) << " schedule";
    sc.schedule(&run_in_fiber);
    sleep(5);
    sc.stop();
    LIM_LOG_INFO(g_logger) << " over" << s_count;
}

int main(int argc, char *argv[])
{
    // test(3, true, "test");
    test(4, true, "test");
    return 0;
}