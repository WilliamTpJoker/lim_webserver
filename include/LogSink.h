#pragma once

#include "FileSize.h"
#include "Mutex.h"

#include <memory>
#include <stdio.h>

namespace lim_webserver
{

    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        using MutexType = Spinlock;

    public:
        virtual ~LogSink(){};

        void append(const char *logline, const size_t len);

        inline void flush()
        {
            fflush(m_ptr);
        }

        FILE *getPtr() const { return m_ptr; }

    protected:
        /**
         * @brief 写入指定长度的数据到文件
         */
        virtual size_t write(const char *logline, size_t len);

        FILE *m_ptr = nullptr; // 文件流
        char m_buffer[64 * 1024];
        MutexType m_mutex;
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
        void open(const char *filename, bool append);

        long getFileSize();

    private:
        size_t write(const char *logline, size_t len) override;

        FileSize::ptr m_fileSize = FileSize::Create(); // 当前已存储的数据量
    };

} // namespace lim_webserver
