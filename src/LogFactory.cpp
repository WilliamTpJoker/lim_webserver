#include "LogFactory.h"
#include "Config.h"

namespace lim_webserver
{
    // 读取配置
    ConfigVar<LogConfigDefine>::ptr g_log_defines = Config::Lookup("logconfig", LogConfigDefine(), "logs config");

    struct LogInitializer
    {
        LogInitializer()
        {
            auto log_listener_func = [](const LogConfigDefine &old_val, const LogConfigDefine &new_val)
            {
                for (auto &lad : new_val.appenders)
                {
                    LogAppender::ptr appender;
                    if (lad.type == 1)
                    {
                        appender = FileAppender::Create(lad.file, lad.append);
                    }
                    else if (lad.type == 0)
                    {
                        appender = ConsoleAppender::Create();
                    }
                    appender->setName(lad.name);
                    appender->setLevel(lad.level);
                    if (!lad.formatter.empty())
                    {
                        LogFormatter::ptr formatter = LogFormatter::Create(lad.formatter);
                        if (formatter->isError())
                        {
                            std::cout << " appender type=" << lad.type << " formatter=" << lad.formatter << " is invalid" << std::endl;
                        }
                        else
                        {
                            appender->setFormatter(formatter);
                        }
                    }
                }

                // for (auto &ld : new_val.loggers)
                // {
                //     Logger::ptr logger;
                //     auto it = old_val.loggers.find(ld);
                //     if (it == old_val.end() || (i != *it))
                //     {
                //         // 新增或修改
                //         logger = LOG_NAME(i.name);
                //     }
                //     logger->setLevel(i.level);
                //     logger->clearAppender();

                // for (auto &i : old_val)
                // {
                //     auto it = new_val.find(i);
                //     if (it == new_val.end())
                //     {
                //         // 删除Logger: 删除其所有Appender,并设置日志级别为最高
                //         auto logger = LOG_NAME(i.name);
                //         logger->clearAppender();
                //         logger->setLevel(LogLevel::OFF);
                //     }
                // }
            };

            g_log_defines->addListener(log_listener_func);
        }
    };

    static LogInitializer __log__init;
} // namespace lim_webserver
