#include <iostream>
#include "log.h"
#include "util.h"

int main(int argc, char *argv[])
{
    using namespace lim_webserver;
    auto logger = MakeShared<Logger>();
    logger->setFormatter(LIM_DEFAULT_PATTERN);
    logger->addAppender(MakeShared<FileLogAppender>("log.txt"));

    auto event = MakeShared<LogEvent>(
        logger, __FILE__, __LINE__, 0,
        lim_webserver::GetThreadId(),
        lim_webserver::GetFiberId(),
        time(0));
    logger->log(lim_webserver::LogLevel::DEBUG, event);
    
    LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::DEBUG);
    LIM_LOG_FATAL(logger);

    auto l = lim_webserver::LoggerMgr::GetInstance()->getLogger("XX");
    LIM_LOG_DEBUG(l);
    return 0;
}