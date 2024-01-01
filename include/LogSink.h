#pragma once

#include <memory>
#include <stdio.h>

namespace lim_webserver
{

    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;

    public:
        virtual ~LogSink(){};

        virtual void append(const char *logline, const size_t len);

        virtual void flush();

        FILE *getPtr() { return m_ptr; }

    protected:
        /**
         * @brief 写入指定长度的数据到文件
         */
        size_t write(const char *logline, size_t len);

        FILE *m_ptr = nullptr; // 文件流
        char m_buffer[64 * 1024];
    };

    class ConsoleSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<ConsoleSink>;

    public:
        ConsoleSink();
    };

    class FileSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<FileSink>;

    public:
        FileSink(const char *filename, bool append);
    };

} // namespace lim_webserver
