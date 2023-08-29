#include "lim.h"

lim_webserver::Shared_ptr<lim_webserver::Logger> g_logger = LIM_LOG_ROOT();
lim_webserver::Shared_ptr<lim_webserver::Logger> g_l = LIM_LOG_NAME("system");
static int s_count = 50000;

void run_in_fiber()
{
    LIM_LOG_INFO(g_logger) << "test in fiber, s_count=" << --s_count;
    sleep(1);
}

void test(int threads, bool use_caller, std::string name)
{
    std::vector<std::function<void()>> f;
    for (int i = 1; i < 5; ++i)
    {
        f.push_back(&run_in_fiber);
    }
    LIM_LOG_INFO(g_logger) << " main";
    lim_webserver::Scheduler sc(threads, use_caller, name);
    sc.start();
    LIM_LOG_INFO(g_logger) << " schedule";
    sc.schedule(f.begin(), f.end());
    sc.stop();
    LIM_LOG_INFO(g_logger) << " over" << s_count;
}

int main(int argc, char *argv[])
{
    g_l->setLevel(LogLevel_INFO);
    // test(3, true, "test");
    test(3, false, "test");
    return 0;
}