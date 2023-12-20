#include <iostream>
#include "log.h"
#include "util.h"

using namespace lim_webserver;

void test_logger()
{
    auto logger = Logger::Create();
    logger->setFormatter(LIM_DEFAULT_PATTERN);
    logger->addAppender(FileLogAppender::Create("log/log.txt"));

    auto event = LogEvent::Create(
        logger, __FILE__, __LINE__, 0,
        time(0),
        LogLevel::DEBUG);
    logger->log(LogLevel::DEBUG, event);

    LIM_LOG_LEVEL(logger, LogLevel::DEBUG) << " test log: support operator<<";
    LIM_LOG_FATAL(logger);
}

void test_new_logger()
{
    Logger::ptr l = LoggerMgr::GetInstance()->getLogger("XX");
    LIM_LOG_DEBUG(l) << " XXX";
    LIM_LOG_INFO(LIM_LOG_NAME("XX")) << " this msg from XX logger";
}

void stress_test(bool longLog, int round, int kBatch)
{
    Logger::ptr logger = LIM_LOG_NAME("test");
    FileLogAppender::ptr appender= FileLogAppender::Create("./log/stress_log.txt");
    logger->addAppender(appender);
    // logger->addAppender(StdoutLogAppender::Create());

    int cnt = 0;
    std::string empty = " ";
    std::string longStr(3000,'X');
    clock_t start = clock();
    for(int t = 0; t < round; ++t){
        for(int i = 0; i < kBatch; ++i){
            LIM_LOG_INFO(logger) << "Hello 123456789" << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
    }
    clock_t end = clock();
    printf("%lf\n", (float)(end-start)*1000 / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[])
{
    // test_logger();
    // test_new_logger();
    printf("program started, pid = %d\n", getpid()),
    stress_test(false, 30, 1000);
    stress_test(true, 30, 1000);

    return 0;
}