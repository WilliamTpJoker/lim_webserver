#include "LogAppender.h"
#include "config.h"

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

    std::string StdoutLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    std::string FileLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename), m_ptr(nullptr)
    {
        reopen();
    }

    void FileLogAppender::log(LogLevel level, LogMessage::ptr event)
    {
        if (level >= m_level)
        {
            if (!fileExist())
            {
                std::cout << "file: " << m_filename << " deleted, Create new one" << std::endl;
                reopen();
            }
            LogStream logstream;
            {
                MutexType::Lock lock(m_mutex);
                m_formatter->format(logstream, event);
            }
            log(level, logstream);
        }
    }

    void FileLogAppender::log(LogLevel level, LogStream &stream)
    {
        if (level >= m_level)
        {
            const LogStream::Buffer &buf(stream.buffer());
            fwrite(buf.data(), 1, buf.length(), m_ptr);
        }
    }

    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);
        if (m_ptr)
        {
            fclose(m_ptr);
        }
        m_ptr = fopen(m_filename.c_str(), "w");
        return !!m_ptr;
    }

    bool FileLogAppender::fileExist()
    {
        std::ifstream file(m_filename);
        return file.good();
    }

    void StdoutLogAppender::log(LogLevel level, LogMessage::ptr event)
    {
        if (level >= m_level)
        {
            LogStream logstream;
            {
                MutexType::Lock lock(m_mutex);
                m_formatter->format(logstream, event);
            }
            log(level, logstream);
        }
    }
    void StdoutLogAppender::log(LogLevel level, LogStream &stream)
    {
        if (level >= m_level)
        {
            const LogStream::Buffer &buf(stream.buffer());
            fwrite(buf.data(), 1, buf.length(), stdout);
        }
    }
} // namespace lim_webserver
