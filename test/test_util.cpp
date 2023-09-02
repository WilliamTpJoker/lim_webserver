#include "lim.h"
#include "macro.h"

lim_webserver::Logger::ptr g_logger = LIM_LOG_ROOT();

void test_backtrace()
{
    LIM_LOG_INFO(g_logger) << lim_webserver::BackTraceToString(10);
}

void test_assert()
{
    LIM_ASSERT(false,"abc","cde");
}

int main(int argc, char **argv)
{
    // test_backtrace();
    test_assert();

    return 0;
}