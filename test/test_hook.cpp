#include "hook.h"
#include "io_manager.h"

lim_webserver::Logger::ptr g_logger = LIM_LOG_ROOT();

// 通过hook将异步的事件转换为同步的事件
void test_sleep()
{
    lim_webserver::IoManager iom(1);
    iom.schedule([](){
        sleep(2);
        LIM_LOG_INFO(g_logger)<<"sleep 2";
    });
    iom.schedule([](){
        sleep(3);
        LIM_LOG_INFO(g_logger)<<"sleep 3";
    });
    LIM_LOG_INFO(g_logger)<<"test sleep";
}

int main(int argc, char *args[])
{
    test_sleep();

    return 0;
}