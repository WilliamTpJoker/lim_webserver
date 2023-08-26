#include "lim.h"

lim_webserver::Shared_ptr<lim_webserver::Logger> g_logger = LIM_LOG_ROOT();

void run_in_fiber()
{
    LIM_LOG_INFO(g_logger) << "run_in_fiber begin";
    lim_webserver::Fiber::YieldToHold();
    LIM_LOG_INFO(g_logger) << "run_in_fiber end";
    lim_webserver::Fiber::YieldToHold();
}

void test_fiber()
{
    LIM_LOG_INFO(g_logger) << "main begin -1";
    {
        lim_webserver::Fiber::GetThis();
        LIM_LOG_INFO(g_logger) << "main begin";
        lim_webserver::Shared_ptr<lim_webserver::Fiber> fiber = lim_webserver::MakeShared<lim_webserver::Fiber>(run_in_fiber);
        fiber->swapIn();
        LIM_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        LIM_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    LIM_LOG_INFO(g_logger) << "main after end2";
}

void test_thread()
{
    std::vector<std::shared_ptr<lim_webserver::Thread>> thread_vec;
    for(int i=0;i<2;++i)
    {
        thread_vec.emplace_back(std::make_shared<lim_webserver::Thread>(&test_fiber,"name_"+std::to_string(i)));
    }
    for(auto i:thread_vec)
    {
        i->join();
    }
}

int main(int argc, char **argv)
{
    lim_webserver::Thread::SetName("main");
    test_thread();
    return 0;
}