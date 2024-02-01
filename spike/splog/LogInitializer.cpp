#include "LogInitializer.h"

namespace lim_webserver
{
    // 读取配置
    static typename ConfigerVar<LogConfigerDefine>::ptr g_log_defines = Configer::Lookup("logconfig", LogConfigerDefine(), "logs config");

    class LogInitializer
    {
    public:
        LogInitializer()
        {
            auto log_listener_func = [](const LogConfigerDefine &old_val, const LogConfigerDefine &new_val)
            {
                auto logManager = LogManager::GetInstance();
                std::vector<LogAppenderDefine> new_appenders = new_val.appenders;
                std::vector<LogAppenderDefine> old_appenders = old_val.appenders;
                std::sort(new_appenders.begin(), new_appenders.end());
                std::sort(old_appenders.begin(), old_appenders.end());
                auto it1 = new_appenders.begin();
                auto it2 = old_appenders.begin();
                while (it1 != new_appenders.end() && it2 != old_appenders.end())
                {
                    // 排序后，新的比旧的小，表示新的有旧的没有，为新增
                    if (*it1 < *it2)
                    {
                        logManager->createOrUpdateAppender(*it1);
                        ++it1;
                    }
                    else if (*it2 < *it1) // 旧的比新的小，表示旧的有新的没有，为删除
                    {
                        // TODO：
                        ++it2;
                    }
                    else // 共有的部分，改动配置
                    {
                        logManager->createOrUpdateAppender(*it1);
                        ++it1;
                        ++it2;
                    }
                }
                for (; it1 != new_appenders.end(); ++it1)
                {
                    logManager->createOrUpdateAppender(*it1);
                }
                for (; it2 != old_appenders.end(); ++it2)
                {
                    // TODO：
                }

                std::vector<LoggerDefine> new_loggers = new_val.loggers;
                std::vector<LoggerDefine> old_loggers = old_val.loggers;
                std::sort(new_loggers.begin(), new_loggers.end());
                std::sort(old_loggers.begin(), old_loggers.end());
                auto it3 = new_loggers.begin();
                auto it4 = old_loggers.begin();
                while (it3 != new_loggers.end() && it4 != old_loggers.end())
                {
                    // 排序后，新的比旧的小，表示新的有旧的没有，为新增
                    if (*it3 < *it4)
                    {
                        logManager->createOrUpdateLogger(*it3);
                        ++it3;
                    }
                    // 旧的比新的小，表示旧的有新的没有，为删除
                    if (*it4 < *it3) 
                    {
                        logManager->delLogger(it4->name);
                        ++it4;
                    }
                    else // 共有的部分，改动配置
                    {
                        logManager->createOrUpdateLogger(*it3);
                        ++it3;
                        ++it4;
                    }
                }
                for (; it3 != new_loggers.end(); ++it3)
                {
                    logManager->createOrUpdateLogger(*it3);
                }
                for (; it4 != old_loggers.end(); ++it4)
                {
                    logManager->delLogger(it4->name);
                }
            };

            g_log_defines->addListener(log_listener_func);
        }
    };
    static LogInitializer __log__init;
} // namespace lim_webserver
