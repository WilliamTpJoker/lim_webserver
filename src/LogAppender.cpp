#include "LogAppender.h"
#include "LogVisitor.h"
#include "Config.h"

#include <iostream>

namespace lim_webserver
{
    void LogAppender::setFormatter(const std::string &pattern)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = LogFormatter::Create(pattern);
    }

    void LogAppender::setFormatter(LogFormatter::ptr formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = formatter;
    }

    const LogFormatter::ptr &LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    void LogAppender::delFormatter()
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = nullptr;
    }

    void OutputAppender::log(LogLevel level, LogMessage::ptr message)
    {
        if (level >= m_level)
        {
            LogStream logstream;
            {
                MutexType::Lock lock(m_mutex);
                m_formatter->format(logstream, message);
            }
            log(level, logstream);
        }
    }

    void OutputAppender::log(LogLevel level, LogStream &stream)
    {
        if (level >= m_level)
        {
            const LogStream::Buffer &buf(stream.buffer());
            fwrite(buf.data(), 1, buf.length(), m_ptr);
        }
    }

    ConsoleAppender::ConsoleAppender(FILE *target)
    {
        setTarget(target);
    }

    void ConsoleAppender::setTarget(FILE *target)
    {
        MutexType::Lock lock(m_mutex);
        if (fileno(target) != fileno(stdout) | fileno(target) != fileno(stdout))
        {
            std::cout << "ConsoleAppender error: wrong target, default set stdout" << std::endl;
            m_ptr = stdout;
        }
        else
        {
            m_ptr = target;
        }
    }

    const char* ConsoleAppender::accept(LogVisitor &visitor)
    {
        return visitor.visitConsoleAppender(*this);
    }

    FileAppender::FileAppender(const std::string &filename, bool append)
        : m_filename(filename), m_append(append)
    {
        reopen();
    }

    bool FileAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);

        if (m_ptr)
        {
            fclose(m_ptr);
        }
        const char *mode = m_append ? "a" : "w";
        m_ptr = fopen(m_filename.c_str(), mode);
        if (!m_ptr)
        {
            std::cout << "FileAppender error: file open failed" << std::endl;
        }
        return !!m_ptr;
    }

    const char* FileAppender::accept(LogVisitor &visitor)
    {
        return visitor.visitFileAppender(*this);
    }

    RollingFileAppender::RollingFileAppender(const std::string &filename, RollingPolicy::ptr rollingPolicy)
        : FileAppender(filename, true), m_rollingPolicy(rollingPolicy)
    {
    }

} // namespace lim_webserver
