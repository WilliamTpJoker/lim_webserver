#include "coroutine/Timer.h"
#include "splog/splog.h"

using namespace lim_webserver;

static Logger::ptr g_logger = LOG_NAME("test");

void test_timer()
{
    TimerManager *tm = TimerManager::GetInstance();


    // 测试同时输出
    for (int i = 0; i < 10; ++i)
    {
        tm->addTimer(1000, []
                     { LOG_INFO(g_logger) << "hello world 1000"; });
    }

    // 测试添加较早超时任务
    tm->addTimer(500, []
                 { LOG_INFO(g_logger) << "hello world 500"; });

    // 测试循环定时任务
    auto timer = tm->addTimer(
        200, []
        { LOG_INFO(g_logger) << "hello world 200"; },
        true);
    
    tm->addTimer(2000, []
                 { LOG_INFO(g_logger) << "hello world 2000"; });

    sleep(1);
    timer->cancel();
    sleep(1);
}

int main()
{
    auto appender = AppenderFactory::GetInstance()->defaultConsoleAppender();
    appender->setFormatter("%d%T%t %N%T[%c] [%p] %f:%l%T%m%n");
    g_logger->addAppender(appender);

    LOG_INFO(g_logger) << "start";
    test_timer();

    return 0;
}