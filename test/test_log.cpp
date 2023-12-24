#include <iostream>
#include "SpikeLog.h"

using namespace lim_webserver;

void test_logger()
{
    Logger::ptr logger = Logger::Create();
    LogAppender::ptr appender = FileAppender::Create("log/log.txt");
    appender->setFormatter(DEFAULT_PATTERN);

    logger->addAppender(FileAppender::Create("log/log.txt"));

    auto event = LogMessage::Create(
        logger, __FILE__, __LINE__, time(0), LogLevel::DEBUG, logger->getName());
    logger->log(LogLevel::DEBUG, event);

    LOG_LEVEL(logger, LogLevel::DEBUG) << " test log: support operator<<";
    LOG_FATAL(logger);
}

void test_new_logger()
{
    Logger::ptr l = LoggerMgr::GetInstance()->getLogger("XX");
    LOG_DEBUG(l) << " XXX";
    LOG_INFO(LOG_NAME("XX")) << " this msg from XX logger";
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
    Logger::ptr logger = LOG_NAME("test");
    FileAppender::ptr appender = FileAppender::Create("./log/stress_log.txt", false);
    appender->setFormatter("%t %N%T%F%T[%c] [%p] %f:%l%T%m%n");
    logger->addAppender(appender);

    stress_test(logger, false, 300, 1000);
    stress_test(logger, true, 300, 1000);
}

int main(int argc, char *argv[])
{
    // test_logger();
    // test_new_logger();
    printf("program started, pid = %d\n", getpid());
    
    test_stress();

    return 0;
}