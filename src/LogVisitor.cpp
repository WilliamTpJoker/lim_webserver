#include "LogVisitor.h"
#include "Logger.h"

#include <sstream>


namespace lim_webserver
{
    const char* YamlVisitor::visitLogger(Logger& logger)
    {
        YAML::Node node;
        node["name"] = logger.m_name;
        LogLevel level = logger.m_level;
        if (level != LogLevel_UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(level);
        }
        return getString(node);
    }

    const char* YamlVisitor::visitConsoleAppender(ConsoleAppender &appender)
    {
        YAML::Node node;
        node["name"] = appender.m_name;
        node["type"] = "ConsoleAppender";
        if (appender.m_level != LogLevel_UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(appender.m_level);
        }
        node["pattern"] = appender.m_formatter->getPattern();
        return getString(node);
    }

    const char* YamlVisitor::visitFileAppender(FileAppender &appender)
    {
        YAML::Node node;
        node["name"] = appender.m_name;
        node["file"] = appender.m_filename;
        node["type"] = "FileAppender";
        if (appender.m_level != LogLevel_UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(appender.m_level);
        }
        node["pattern"] = appender.m_formatter->getPattern();
        node["append"] = appender.m_append;
        return getString(node);
    }

    const char* YamlVisitor::getString(YAML::Node &node)
    {
        MutexType::Lock lock(m_mutex);
        m_emitter<<node<<YAML::EndMap;
        return m_emitter.c_str();
    }

} // namespace lim_webserver
