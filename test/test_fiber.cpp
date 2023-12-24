#include "lim.h"

lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void run_in_fiber()
{
    LOG_INFO(g_logger) << "run_in_fiber begin";
    lim_webserver::Fiber::YieldToHold();
    LOG_INFO(g_logger) << "run_in_fiber end";
    lim_webserver::Fiber::YieldToHold();
}

void test_fiber()
{
    LOG_INFO(g_logger) << "main begin -1";
    {
        lim_webserver::Fiber::GetThis();
        LOG_INFO(g_logger) << "main begin";
        lim_webserver::Fiber::ptr fiber = lim_webserver::Fiber::Create(run_in_fiber,0,true);
        fiber->call();
        LOG_INFO(g_logger) << "main after call";
        fiber->call();
        LOG_INFO(g_logger) << "main after end";
        fiber->call();
    }
    LOG_INFO(g_logger) << "main after end2";
}

void test_thread()
{
    std::vector<std::shared_ptr<lim_webserver::Thread>> thread_vec;
    for(int i=0;i<1;++i)
    {
        thread_vec.emplace_back(lim_webserver::Thread::Create(&test_fiber,"name_"+std::to_string(i)));
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