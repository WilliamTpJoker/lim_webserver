#include "SpikeLog.h"


using namespace lim_webserver;

static Logger::ptr g_logger = LOG_ROOT();

void test_visit()
{
    YamlVisitor visitor;

    LOG_INFO(g_logger)<<g_logger->accept(visitor);
    LOG_INFO(g_logger)<<g_logger->accept(visitor);
}


int main()
{
    test_visit();
    return 0;
}