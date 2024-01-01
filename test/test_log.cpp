#include <iostream>
#include "SpikeLog.h"

using namespace lim_webserver;

void test_logger()
{
    LOG_DEBUG(LOG_ROOT()) << " test log: debug test";
    LOG_INFO(LOG_ROOT()) << " test log: info test";
    LOG_ERROR(LOG_ROOT()) << "test log: error test";
}

void test_file_append()
{
    Logger::ptr logger = LOG_NAME("test");
    FileAppender::ptr appender = FileAppender::ptr(new FileAppender("/home/book/Webserver/log/test_log.txt"));
    appender->setName("file_test");
    appender->setFormatter(DEFAULT_PATTERN);
    appender->setAppend(false);
    appender->start();

    logger->addAppender(appender);
    LOG_DEBUG(logger) << "test file log: debug test";
    // appender->stop();
}

void test_async_appender()
{
    Logger::ptr logger = LOG_NAME("test");
    ConsoleAppender::ptr appender = ConsoleAppender::ptr(new ConsoleAppender());
    appender->setName("console_test");
    appender->setFormatter(DEFAULT_PATTERN);
    appender->start();

    FileAppender::ptr fappender = FileAppender::ptr(new FileAppender("/home/book/Webserver/log/test_log.txt"));
    fappender->setName("file_test");
    fappender->setFormatter(DEFAULT_PATTERN);
    fappender->setAppend(false);
    fappender->start();
    logger->addAppender(fappender);

    AsyncAppender::ptr asy_appender = AsyncAppender::ptr(new AsyncAppender(appender));
    asy_appender->setInterval(1);
    asy_appender->start();
    logger->addAppender(asy_appender);

    LOG_DEBUG(logger) << "test async log: debug test";
    LOG_DEBUG(logger) << "test async log: debug test2";
    sleep(3);
    asy_appender->stop();
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
    std::cout<<(float)(end - start) * 1000 / CLOCKS_PER_SEC<<std::endl;
}

void test_stress()
{
    Logger::ptr logger = LOG_NAME("test");

    FileAppender::ptr fappender = FileAppender::ptr(new FileAppender("/home/book/Webserver/log/stress_log.txt"));
    fappender->setName("file_test");
    fappender->setFormatter("%t %N%T%F%T[%c] [%p] %f:%l%T%m%n");
    fappender->setAppend(false);
    fappender->start();
    fappender->setLevel(LogLevel_DEBUG);

    AsyncAppender::ptr asy_appender = AsyncAppender::ptr(new AsyncAppender(fappender));
    asy_appender->setInterval(2);
    // asy_appender->start();

    logger->addAppender(fappender);

    stress_test(logger, false, 30, 1000);
    stress_test(logger, true, 30, 1000);
    // asy_appender->stop();
}

int main(int argc, char *argv[])
{
    // test_logger();
    // test_file_append();
    // test_async_appender();
    // test_new_logger();
    test_stress();

    return 0;
}