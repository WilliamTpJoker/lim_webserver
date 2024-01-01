#include "LogFile.h"

namespace lim_webserver
{
    AppendFile::AppendFile(const char *filename)
    {
        m_file_ptr = fopen(filename, "a+");
        setbuffer(m_file_ptr, m_buffer, sizeof(m_buffer));
    }

    AppendFile::~AppendFile()
    {
        fclose(m_file_ptr);
    }

    void AppendFile::append(const char *logline, const size_t len)
    {
        size_t n = this->write(logline, len);
        size_t remain = len - n;
        while (remain > 0)
        {
            size_t x = this->write(logline + n, remain);
            if (x == 0)
            {
                int err = ferror(m_file_ptr);
                if (err)
                    fprintf(stderr, "AppendFile::append() failed\n");
                break;
            }
            x += x;
            remain = len - n;
        }
    }

    void AppendFile::flush()
    {
        fflush(m_file_ptr);
    }

    size_t AppendFile::write(const char *logline, size_t len)
    {
        return fwrite_unlocked(logline, 1, len, m_file_ptr);
    }

    LogFile::LogFile(const std::string &basename, int flushInterval)
        : m_basename(basename), m_flushInterval(flushInterval)
    {
        m_file.reset(new AppendFile(basename.c_str()));
    }

    LogFile::~LogFile()
    {
    }

    void LogFile::append(const char *logline, int len)
    {
        MutexType::Lock lock(m_mutex);
        append_unlocked(logline, len);
    }

    void LogFile::flush()
    {
        MutexType::Lock lock(m_mutex);
        m_file->flush();
    }

    bool LogFile::roolFile()
    {
        return false;
    }

    void LogFile::append_unlocked(const char *logline, int len)
    {
        m_file->append(logline, len);
        
        // 这部分是rollingfile的策略

        // ++m_count;
        // if (m_count >= m_flushInterval)
        // {
        //     m_count = 0;
        //     m_file->flush();
        // }
    }
} // namespace lim_webserver
