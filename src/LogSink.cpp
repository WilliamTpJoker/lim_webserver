#include "LogSink.h"

namespace lim_webserver
{
    void LogSink::append(const char *logline, const size_t len)
    {
        size_t n = write(logline, len);
        // size_t remain = len - n;
        // while (remain > 0)
        // {
        //     size_t x = write(logline + n, remain);
        //     if (x == 0)
        //     {
        //         int err = ferror(m_ptr);
        //         if (err)
        //             fprintf(stderr, "AppendFile::append() failed\n");
        //         break;
        //     }
        //     x += x;
        //     remain = len - n;
        // }
    }

    void LogSink::flush()
    {
        fflush(m_ptr);
    }

    size_t LogSink::write(const char *logline, size_t len)
    {
        fwrite_unlocked(logline, 1, len, m_ptr);
    }

    ConsoleSink::ConsoleSink()
    {
        m_ptr = stdout;
        setbuffer(m_ptr, m_buffer, sizeof(m_buffer));
    }

    FileSink::FileSink(const char *filename, bool append)
    {
        m_ptr = fopen(filename, append ? "a+" : "w+");
        setbuffer(m_ptr, m_buffer, sizeof(m_buffer));
    }

} // namespace lim_webserver
