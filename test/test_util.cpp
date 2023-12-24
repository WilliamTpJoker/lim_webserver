#include "SpikeLog.h"

lim_webserver::Logger::ptr g_logger = LOG_ROOT();

void test_backtrace()
{
    LOG_INFO(g_logger) << lim_webserver::BackTraceToString(10);
}

void test_assert()
{
    ASSERT(false,"abc","cde");
}

int main(int argc, char **argv)
{
    // test_backtrace();
    test_assert();

    return 0;
}