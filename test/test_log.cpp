#include <iostream>
#include "SpikeLog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_NAME("test");

FileAppender::ptr build_fileAppender()
{
    FileAppender::ptr appender = FileAppender::Create();
    appender->setFile("/home/book/Webserver/log/test_log.txt");
    appender->setName("file_test");
    appender->setFormatter(DEFAULT_PATTERN);
    appender->setAppend(false);
    appender->start();
    return appender;
}

ConsoleAppender::ptr build_consoleAppender()
{
    ConsoleAppender::ptr appender = ConsoleAppender::Create();
    appender->setName("console_test");
    appender->setFormatter("%m%n");
    appender->start();
    return appender;
}

AsyncAppender::ptr build_asyncAppender(OutputAppender::ptr appender)
{
    AsyncAppender::ptr asy_appender = AsyncAppender::Create();
    asy_appender->bindAppender(appender);
    asy_appender->setInterval(1);
    asy_appender->start();
    return asy_appender;
}

void test_logger()
{
    LOG_DEBUG(LOG_ROOT()) << " test log: debug test";
    LOG_INFO(LOG_ROOT()) << " test log: info test";
    LOG_ERROR(LOG_ROOT()) << "test log: error test";
}

void test_file_append()
{
    Logger::ptr logger = LOG_NAME("test");
    FileAppender::ptr appender = build_fileAppender();

    logger->addAppender(appender);
    LOG_DEBUG(logger) << "test file log: debug test";
    // appender->stop();
}

void test_async_appender()
{
    Logger::ptr logger = LOG_NAME("test");
    ConsoleAppender::ptr appender = build_consoleAppender();

    FileAppender::ptr fappender = build_fileAppender();

    logger->addAppender(fappender);

    AsyncAppender::ptr asy_appender = build_asyncAppender(appender);

    logger->addAppender(asy_appender);

    for(int i=0;i<500;++i)
    {
        LOG_DEBUG(logger) << "test async log: debug test "<<i;
    }
    asy_appender->stop();
}

int main(int argc, char *argv[])
{
    // test_logger();
    // test_file_append();
    test_async_appender();

    return 0;
}