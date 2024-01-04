#include "LogSink.h"

namespace lim_webserver
{
    void LogSink::append(const char *logline, const size_t len)
    {
        size_t n = write(logline, len);
        size_t remain = len - n;
        while (remain > 0)
        {
            size_t x = write(logline + n, remain);
            if (x == 0)
            {
                int err = ferror(m_ptr);
                if (err)
                    fprintf(stderr, "AppendFile::append() failed\n");
                break;
            }
            x += x;
            remain = len - n;
        }
    }

    void LogSink::flush()
    {
        fflush(m_ptr);
    }

    size_t LogSink::write(const char *logline, size_t len)
    {
        return fwrite_unlocked(logline, 1, len, m_ptr);
    }

    ConsoleSink::ConsoleSink()
    {
        m_ptr = stdout;
        setbuffer(m_ptr, m_buffer, sizeof(m_buffer));
    }

    FileSink::FileSink(const char *filename, bool append)
    {
        const char* mode = append ? "a+" : "w+";
        m_ptr = fopen(filename, mode);
        setbuffer(m_ptr, m_buffer, sizeof(m_buffer));
    }

    long FileSink::getFileSize()
    {
        return m_fileSize->getSize();
    }

    size_t FileSink::write(const char *logline, size_t len)
    {
        size_t rt = LogSink::write(logline,len);
        m_fileSize->addSize(rt);
        return rt;
    }

} // namespace lim_webserver
