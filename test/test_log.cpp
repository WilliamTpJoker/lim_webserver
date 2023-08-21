#include <iostream>
#include "log.h"
#include "util.h"

int main(int argc, char *argv[])
{
    using namespace lim_webserver;
    auto logger = MakeShared<Logger>();
    logger->setFormatter(LIM_DEFAULT_PATTERN);
    logger->addAppender(MakeShared<FileLogAppender>("log/log.txt"));

    auto event = MakeShared<LogEvent>(
        logger, __FILE__, __LINE__, 0,
        lim_webserver::GetThreadId(),
        lim_webserver::GetFiberId(),
        time(0));
    logger->log(lim_webserver::LogLevel::DEBUG, event);
    
    LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::DEBUG)<<" test log: support operator<<";
    LIM_LOG_FATAL(logger);

    lim_webserver::Shared_ptr<lim_webserver::Logger> l = lim_webserver::LoggerMgr::GetInstance()->getLogger("XX");
    LIM_LOG_DEBUG(l)<<" XXX";

    LIM_LOG_INFO(LIM_LOG_NAME("XX"))<<" this msg from XX logger";

    return 0;
}