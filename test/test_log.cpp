#include <iostream>
#include "splog.h"

using namespace lim_webserver;

static lim_webserver::Logger::ptr g_logger = LOG_NAME("test");
static lim_webserver::Logger::ptr g_logger_async = LOG_NAME("test_async");
static lim_webserver::Logger::ptr g_logger_async2 = LOG_NAME("test_async2");

AsyncAppender::ptr build_asyncAppender(OutputAppender::ptr appender)
{
    AsyncAppender::ptr asy_appender = AppenderFactory::newAsyncAppender();
    asy_appender->bindAppender(appender);
    asy_appender->setInterval(2000);
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
}

void test_appender2()
{
    for (int i = 0; i < 500; ++i)
    {
        LOG_DEBUG(g_logger_async) << "test "<<g_logger_async->getName()<<": debug test " << i;
        LOG_DEBUG(g_logger_async2) << "test "<<g_logger_async2->getName()<<": debug test " << i;
    }
}

int main(int argc, char *argv[])
{
    ConsoleAppender::ptr appender =AppenderFactory::GetInstance()->defaultConsoleAppender();

    FileAppender::ptr fappender =AppenderFactory::GetInstance()->defaultFileAppender();
    fappender->setFile("/home/book/Webserver/log/test_log.txt");
    fappender->setAppend(false);

    AsyncAppender::ptr asy_appender = build_asyncAppender(appender);

    g_logger->addAppender(appender);
    g_logger->addAppender(fappender);

    g_logger_async->addAppender(fappender);
    g_logger_async->addAppender(asy_appender);

    g_logger_async2->addAppender(fappender);
    g_logger_async2->addAppender(asy_appender);

    test_logger();
    test_file_append();
    test_appender(g_logger);
    test_appender(g_logger_async);
    test_appender(g_logger_async2);

    test_appender2();
    asy_appender->stop();
    return 0;
}