#include <iostream>
#include "SpikeLog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_NAME("test");
static lim_webserver::Logger::ptr g_logger_async = LOG_NAME("test_async");

AsyncAppender::ptr build_asyncAppender(OutputAppender::ptr appender)
{
    AsyncAppender::ptr asy_appender = AppenderFactory::newAsyncAppender();
    asy_appender->bindAppender(appender);
    asy_appender->setInterval(2);
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
    LOG_DEBUG(g_logger) << "test file log: debug test";
}

void test_appender(Logger::ptr logger)
{
    for (int i = 0; i < 500; ++i)
    {
        LOG_DEBUG(logger) << "test "<<logger->getName()<<": debug test " << i;
    }
    sleep(1);

    LOG_DEBUG(logger) << "test "<<logger->getName()<<": end";
}

int main(int argc, char *argv[])
{
    ConsoleAppender::ptr appender =AppenderFcty::GetInstance()->defaultConsoleAppender();

    FileAppender::ptr fappender =AppenderFcty::GetInstance()->defaultFileAppender();
    fappender->setFile("/home/book/Webserver/log/test_log.txt");
    fappender->setAppend(false);

    AsyncAppender::ptr asy_appender = build_asyncAppender(appender);

    g_logger->addAppender(appender);
    g_logger->addAppender(fappender);

    g_logger_async->addAppender(fappender);
    g_logger_async->addAppender(asy_appender);

    // test_logger();
    // test_file_append();
    test_appender(g_logger);
    test_appender(g_logger_async);

    asy_appender->stop();
    return 0;
}