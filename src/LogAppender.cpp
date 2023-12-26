#include "LogAppender.h"
#include "LogVisitor.h"
#include "LogManager.h"
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

    OutputAppender::OutputAppender(const LogAppenderDefine &lad)
    {
        m_name=lad.name;
        m_level = lad.level;
        m_formatter = LogFormatter::Create(lad.formatter);
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

    ConsoleAppender::ConsoleAppender()
    {
        m_ptr = stdout;
    }

    ConsoleAppender::ConsoleAppender(const LogAppenderDefine &lad)
    :OutputAppender(lad)
    {
        m_ptr = stdout;
    }

    int ConsoleAppender::getType()
    {
        return 0;
    }

    const char *ConsoleAppender::accept(LogVisitor &visitor)
    {
        return visitor.visitConsoleAppender(*this);
    }

    FileAppender::FileAppender(const std::string &filename, bool append)
        : m_filename(filename), m_append(append)
    {
        reopen();
    }

    FileAppender::FileAppender(const LogAppenderDefine &lad)
    : OutputAppender(lad)
    {
        m_append = lad.append;
        m_filename = lad.file;
        reopen();
    }

    bool FileAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);

        if (m_ptr)
        {
            fclose(m_ptr);
        }
        std::string mode;
        if(m_append)
        {
            mode = "a";
        }
        else
        {
            mode = "w";
        }
        m_ptr = std::move(fopen(m_filename.c_str(), mode.c_str()));
        if (m_ptr==nullptr)
        {
            std::cout << "FileAppender error: file open failed" << std::endl;
        }
        return !!m_ptr;
    }

    void FileAppender::setFile(const std::string &filename)
    {
        m_filename = filename;
        reopen();
    }

    void FileAppender::setAppend(bool append)
    {
        m_append = append;
    }

    int FileAppender::getType()
    {
        return 1;
    }

    const char *FileAppender::accept(LogVisitor &visitor)
    {
        return visitor.visitFileAppender(*this);
    }

    RollingFileAppender::RollingFileAppender(const std::string &filename, RollingPolicy::ptr rollingPolicy)
        : FileAppender(filename, true), m_rollingPolicy(rollingPolicy)
    {
    }

} // namespace lim_webserver
