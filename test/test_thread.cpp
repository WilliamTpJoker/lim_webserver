#include "thread.h"
#include "log.h"
#include <unistd.h>
#include "config.h"

using namespace lim_webserver;

static Logger::ptr g_logger = LIM_LOG_NAME("system");
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
    std::vector<Thread::ptr> thread_vec;
    for (int i = 0; i < 1; ++i)
    {
        Thread::ptr thr = Thread::create(&func2, "name_" + std::to_string(2 * i));
        Thread::ptr thr2 = Thread::create(&func3, "name_" + std::to_string(2 * i + 1));
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

Mutex mtx;
ConditionVariable<Mutex> cv;
int ready=0;

void func4()
{
    {
        lim_webserver::Mutex::Lock lock(mtx);
        cv.wait(mtx, []
                { std::cout<<ready++<<std::endl;return ready>2; });
    }
    std::cout<< "in func4"<<std::endl;
}

void test_cond()
{
    std::vector<Thread::ptr> thread_vec;
    for (int i = 0; i < 2; ++i)
    {
        Thread::ptr thr = Thread::create(&func4, "name_" + std::to_string(2 * i));
        thread_vec.emplace_back(thr);
    }
    cv.notify_all();
    for (int i = 0; i < thread_vec.size(); ++i)
    {
        thread_vec[i]->join();
    }
}

int main(int argc, char **argv)
{
    // Config::LoadFromYaml("./config/test.yaml");
    // test_log_thread();
    test_cond();
    return 0;
}