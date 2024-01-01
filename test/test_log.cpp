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
    FileAppender::ptr appender = AppenderFactory::Create<FileAppender>();
    appender->setFile("/home/book/Webserver/log/test_log.txt");
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
    ConsoleAppender::ptr appender = AppenderFactory::Create<ConsoleAppender>();
    appender->setName("console_test");
    appender->setFormatter(DEFAULT_PATTERN);
    appender->start();

    FileAppender::ptr fappender = AppenderFactory::Create<FileAppender>();
    fappender->setFile("/home/book/Webserver/log/test_log.txt");
    fappender->setName("file_test");
    fappender->setFormatter(DEFAULT_PATTERN);
    fappender->setAppend(false);
    fappender->start();
    logger->addAppender(fappender);

    AsyncAppender::ptr asy_appender = AppenderFactory::Create<AsyncAppender>();
    asy_appender->bindAppender(appender);
    asy_appender->setInterval(1);
    asy_appender->start();
    logger->addAppender(asy_appender);

    LOG_DEBUG(logger) << "test async log: debug test";
    LOG_DEBUG(logger) << "test async log: debug test2";
    sleep(3);
    asy_appender->stop();
}


int main(int argc, char *argv[])
{
    // test_logger();
    // test_file_append();
    test_async_appender();

    return 0;
}