#include "coroutine/Syntex.h"
#include "coroutine/Timer.h"

// 根调度器
#define g_Scheduler lim_webserver::Scheduler::GetInstance()
#define fiber_sched g_Scheduler

// 创建协程
#define fiber lim_webserver::Syntex() -

// 协程主动让出线程权
#define fiber_yield                             \
    do                                       \
    {                                        \
        lim_webserver::Processor::CoYield(); \
    } while (0)
