#pragma once

#include "Noncopyable.h"
#include "Mutex.h"

namespace lim_webserver
{
    /**
     * @brief 向文件追加内容
     */
    class AppendFile : Noncopyable
    {
    public:
        /**
         * @brief 显示构造函数
         */
        explicit AppendFile(const char *filename);

        /**
         * @brief 析构函数，关闭打开的文件
         */
        ~AppendFile();

        /**
         * @brief 向文件追加指定长度的日志行
         */
        void append(const char *logline, const size_t len);

        /**
         * @brief 刷新文件缓冲区
         */
        void flush();

    private:
        /**
         * @brief 写入指定长度的数据到文件
         */
        size_t write(const char *logline, size_t len);

        FILE *m_file_ptr;         // 文件指针
        char m_buffer[64 * 1024]; // 64k 缓存
    };

    /**
     * @brief 管理打开的文件
     */
    class LogFile : Noncopyable
    {
    public:
        using MutexType = Mutex;

    public:
        /**
         * @brief 构造函数，接受基本文件名和刷新间隔作为参数
         */
        LogFile(const std::string &basename, int flushInterval = 1024);

        /**
         * @brief 析构函数
         */
        ~LogFile();

        /**
         * @brief 向日志文件追加指定长度的日志行
         */
        void append(const char *logline, int len);

        /**
         * @brief 刷新文件缓冲区
         */
        void flush();

        /**
         * @brief 滚动文件
         */
        bool roolFile();

    private:
        /**
         * @brief 无锁方式向日志文件追加指定长度的日志行
         */
        void append_unlocked(const char *logline, int len);

        const std::string m_basename;       // 基本文件名
        const int m_flushInterval;          // 刷新间隔
        int m_count;                        // 记录追加次数
        std::unique_ptr<AppendFile> m_file; // 持有 AppendFile 对象的智能指针
        MutexType m_mutex;
    };
} // namespace lim_webserver
