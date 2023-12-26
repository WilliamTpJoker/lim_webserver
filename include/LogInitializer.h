#pragma once

#include <algorithm>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <vector>

#include "LogManager.h"
#include "Config.h"

namespace lim_webserver
{
    struct LogConfigDefine
    {
        std::vector<LoggerDefine> loggers;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogConfigDefine &oth) const
        {
            return appenders == oth.appenders && loggers == oth.loggers;
        }
    };

    template <>
    class LexicalCast<std::string, LogConfigDefine>
    {
    public:
        LogConfigDefine operator()(const std::string &v)
        {
            YAML::Node logConfigNode = YAML::Load(v);
            LogConfigDefine lcd;
            if (logConfigNode["appenders"].IsDefined())
            {
                for (auto appenderNode : logConfigNode["appenders"])
                {
                    if (!appenderNode["type"].IsDefined())
                    {
                        std::cout << "log config error: appender type is null, " << appenderNode << std::endl;
                        continue;
                    }
                    std::string type = appenderNode["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if (type == "1") // FileAppender
                    {
                        lad.type = 1;
                        if (!appenderNode["file"].IsDefined())
                        {
                            std::cout << "log config error: fileappender file is null, " << appenderNode << std::endl;
                            continue;
                        }
                        lad.file = appenderNode["file"].as<std::string>();
                        if (appenderNode["name"].IsDefined())
                        {
                            lad.name = appenderNode["name"].as<std::string>();
                        }
                        if (appenderNode["append"].IsDefined())
                        {
                            lad.append = appenderNode["append"].as<bool>();
                        }
                        if (appenderNode["level"].IsDefined())
                        {
                            lad.level = LogLevelHandler::FromString(appenderNode["level"].as<std::string>());
                        }
                        if (appenderNode["formatter"].IsDefined())
                        {
                            lad.formatter = appenderNode["formatter"].as<std::string>();
                        }
                    }
                    else if (type == "0")
                    {
                        lad.type = 0;
                        if (appenderNode["level"].IsDefined())
                        {
                            lad.level = LogLevelHandler::FromString(appenderNode["level"].as<std::string>());
                        }
                        if (appenderNode["name"].IsDefined())
                        {
                            lad.name = appenderNode["name"].as<std::string>();
                        }
                        if (appenderNode["formatter"].IsDefined())
                        {
                            lad.formatter = appenderNode["formatter"].as<std::string>();
                        }
                    }
                    else
                    {
                        std::cout << "log config error: appender type is invalid, " << appenderNode << std::endl;
                        continue;
                    }

                    lcd.appenders.push_back(lad);
                }
            }
            if (logConfigNode["loggers"].IsDefined())
            {
                for (auto logNode : logConfigNode["loggers"])
                {
                    LoggerDefine ld;
                    if (!logNode["name"].IsDefined())
                    {
                        std::cout << "log config error: name is null, " << logNode << std::endl;
                        throw std::logic_error("log config name is null");
                    }
                    ld.name = logNode["name"].as<std::string>();
                    ld.level = LogLevelHandler::FromString(logNode["level"].IsDefined() ? logNode["level"].as<std::string>() : "");
                    if (logNode["appender-ref"].IsDefined())
                    {
                        ld.appender_refs = logNode["appender-ref"].as<std::vector<std::string>>();
                    }

                    lcd.loggers.push_back(ld);
                }
            }
            return lcd;
        }
    };

    template <>
    class LexicalCast<LogConfigDefine, std::string>
    {
    public:
        std::string operator()(const LogConfigDefine &lcd)
        {
            YAML::Node logConfigNode;
            for (auto &lad : lcd.appenders)
            {
                YAML::Node appenderNode;
                if (lad.type == 1)
                {
                    appenderNode["type"] = "FileAppender";
                    appenderNode["file"] = lad.file;
                    appenderNode["append"] = lad.append;
                }
                else if (lad.type == 0)
                {
                    appenderNode["type"] = "ConsoleAppender";
                }
                if (lad.level != LogLevel_UNKNOWN)
                {
                    appenderNode["level"] = LogLevelHandler::ToString(lad.level);
                }

                if (!lad.formatter.empty())
                {
                    appenderNode["formatter"] = lad.formatter;
                }
                if (!lad.name.empty())
                {
                    appenderNode["name"] = lad.name;
                }

                logConfigNode["appenders"].push_back(appenderNode);
            }
            for (auto &ld : lcd.loggers)
            {
                YAML::Node logNode;
                logNode["name"] = ld.name;
                if (ld.level != LogLevel_UNKNOWN)
                {
                    logNode["level"] = LogLevelHandler::ToString(ld.level);
                }
                if (!ld.appender_refs.empty())
                {
                    for (auto &appender_ref : ld.appender_refs)
                    {
                        logNode["appender-ref"].push_back(appender_ref);
                    }
                }
                logConfigNode["loggers"].push_back(logNode);
            }

            std::stringstream ss;
            ss << logConfigNode;
            return ss.str();
        }
    };

    // 读取配置
    typename ConfigVar<LogConfigDefine>::ptr g_log_defines = Config::Lookup("logconfig", LogConfigDefine(), "logs config");

    class LogInitializer
    {
    public:
        LogInitializer()
        {
            auto log_listener_func = [](const LogConfigDefine &old_val, const LogConfigDefine &new_val)
            {
                auto logManager = LogMgr::GetInstance();
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
    LogInitializer __log__init;
} // namespace lim_webserver
