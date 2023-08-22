#include "lim.h"
#include <unistd.h>

using namespace lim_webserver;

Shared_ptr<Logger> g_logger = LIM_LOG_ROOT();

void func1()
{
    LIM_LOG_INFO(g_logger) << "name:" << Thread::GetThisThreadName()
                           << " this.name:" << Thread::GetThis()->getName()
                           << " id:" << GetThreadId()
                           << " this.id:" << Thread::GetThis()->getId();
    sleep(20);
}

int main(int argc, char **argv)
{
    LIM_LOG_INFO(g_logger) << "thread_test_begin";
    std::vector<Shared_ptr<Thread>> thread_vec;
    for (int i=0; i < 5; ++i)
    {
        Shared_ptr<Thread> thr = MakeShared<Thread>(&func1, "name_" + std::to_string(i));
        thread_vec.emplace_back(thr);
    }

    for (int i=0; i < 5; ++i)
    {
        thread_vec[i]->join();
    }

    return 0;
}