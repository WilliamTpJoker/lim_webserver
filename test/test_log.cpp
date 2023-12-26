#include <iostream>
#include "SpikeLog.h"

using namespace lim_webserver;

void test_logger()
{
    LOG_DEBUG(LOG_ROOT()) << " test log: debug test";
    LOG_INFO(LOG_ROOT()) << " test log: info test";
    LOG_ERROR(LOG_NAME("system")) << "test log: error test";
}

void stress_test(Logger::ptr &logger, bool longLog, int round, int kBatch)
{
    int cnt = 0;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    clock_t start = clock();
    for (int t = 0; t < round; ++t)
    {
        for (int i = 0; i < kBatch; ++i)
        {
            LOG_INFO(logger) << "Hello 123456789"
                                 << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
    }
    clock_t end = clock();
    printf("%lf\n", (float)(end - start) * 1000 / CLOCKS_PER_SEC);
}

void test_stress()
{
    LogManager* logManager = LogMgr::GetInstance();
    LogAppenderDefine lad;
    lad.name = "test";
    lad.file = "/home/book/Webserver/log/stress_log.txt";
    lad.append = false;
    lad.level = LogLevel_DEBUG;
    lad.type = 1;
    lad.formatter = "%t %N%T%F%T[%c] [%p] %f:%l%T%m%n";

    logManager->createAppender(lad);

    LoggerDefine ld;
    ld.name = "test";
    ld.appender_refs = {"test"};
    Logger::ptr logger = logManager->createLogger(ld);;
    YamlVisitor visitor;
    std::cout<<logger->accept(visitor)<<std::endl;
    stress_test(logger, false, 300, 1000);
    stress_test(logger, true, 300, 1000);
}

int main(int argc, char *argv[])
{
    // test_logger();
    // test_new_logger();
    test_stress();

    return 0;
}