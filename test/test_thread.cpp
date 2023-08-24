#include "thread.h"
#include "log.h"
#include <unistd.h>
#include "config.h"

using namespace lim_webserver;

static Shared_ptr<Logger> g_logger = LIM_LOG_NAME("system");
RWMutex s_rwmutex;
Mutex s_mutex;

int count = 0;

void func1()
{
    LIM_LOG_INFO(g_logger) << "name:" << Thread::GetThisThreadName()
                           << " this.name:" << Thread::GetThis()->getName()
                           << " id:" << GetThreadId()
                           << " this.id:" << Thread::GetThis()->getId();
    for (int i = 0; i < 10000000; ++i)
    {
        // RWMutex::WriteLock lock(s_rwmutex);
        Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void func2()
{
    while (true)
    {
        LIM_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void func3()
{
    while (true)
    {
        LIM_LOG_INFO(g_logger) << "========================================";
    }
}

void test_log_thread()
{
    LIM_LOG_INFO(g_logger) << "thread test begin";
    std::vector<Shared_ptr<Thread>> thread_vec;
    for (int i = 0; i < 2; ++i)
    {
        Shared_ptr<Thread> thr = MakeShared<Thread>(&func2, "name_" + std::to_string(2 * i));
        Shared_ptr<Thread> thr2 = MakeShared<Thread>(&func3, "name_" + std::to_string(2 * i + 1));
        thread_vec.emplace_back(thr);
        thread_vec.emplace_back(thr2);
    }

    for (int i = 0; i < thread_vec.size(); ++i)
    {
        thread_vec[i]->join();
    }
    LIM_LOG_INFO(g_logger) << "thread test end";
    LIM_LOG_INFO(g_logger) << "count=" << count;
}

int main(int argc, char **argv)
{
    Config::LoadFromYaml("./config/test.yaml");
    test_log_thread();
    return 0;
}