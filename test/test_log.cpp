#include <iostream>
#include "log.h"
#include "util.h"

using namespace lim_webserver;

void test_loggor()
{
    auto logger = MakeShared<Logger>();
    logger->setFormatter(LIM_DEFAULT_PATTERN);
    logger->addAppender(MakeShared<FileLogAppender>("log/log.txt"));

    auto event = MakeShared<LogEvent>(
        logger, __FILE__, __LINE__, 0,
        GetThreadId(),
        GetFiberId(),
        time(0));
    logger->log(LogLevel::DEBUG, event);

    LIM_LOG_LEVEL(logger, LogLevel::DEBUG) << " test log: support operator<<";
    LIM_LOG_FATAL(logger);
}

void test_new_logger()
{
    Shared_ptr<Logger> l = LoggerMgr::GetInstance()->getLogger("XX");
    LIM_LOG_DEBUG(l) << " XXX";
    LIM_LOG_INFO(LIM_LOG_NAME("XX")) << " this msg from XX logger";
}

int main(int argc, char *argv[])
{
    // test_loggor();
    test_new_logger();

    return 0;
}