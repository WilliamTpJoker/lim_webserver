#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "Mutex.h"

namespace lim_webserver
{
    class Logger;
    class LogManager;
    class FileAppender;
    class ConsoleAppender;

    class LogVisitor
    {
    public:
        using MutexType = Spinlock;

    public:
        virtual const char *visitLogManager(LogManager &manager) = 0;
        virtual const char *visitLogger(Logger &logger) = 0;
        virtual const char *visitConsoleAppender(ConsoleAppender &appender) = 0;
        virtual const char *visitFileAppender(FileAppender &appender) = 0;

    protected:
        MutexType m_mutex;
    };

    class YamlVisitor : public LogVisitor
    {
    public:
        const char *visitLogManager(LogManager &manager) override;
        const char *visitLogger(Logger &logger) override;
        const char *visitConsoleAppender(ConsoleAppender &appender);
        const char *visitFileAppender(FileAppender &appender);

    private:
        const char *getString(YAML::Node &node);

        YAML::Emitter m_emitter;
    };
} // namespace lim_webserver
