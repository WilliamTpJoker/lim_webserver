#pragma once

#include "base/Singleton.h"
#include "base/Mutex.h"
#include "splog/Logger.h"

namespace lim_webserver
{
    /**
     * @brief 日志事件包装器
     */
    class LogMessageWrap
    {
    public:
        LogMessageWrap(LogMessage::ptr e)
            : m_message(e) {}
        /**
         * @brief 析构时输出日志
         */
        ~LogMessageWrap()
        {
            m_message->getLogger()->log(m_message);
        }

        /**
         * @brief 获得日志事件
         */
        LogMessage::ptr getMessage() const { return m_message; }
        /**
         * @brief 获得日志内容流，以便左移操作补充内容
         */
        LogStream &getStream() { return m_message->getStream(); }

    private:
        LogMessage::ptr m_message; // 事件
    };

    class LogManager:public Singleton<LogManager>
    {
        friend Singleton<LogManager>;
    public:
        using MutexType = Spinlock;

    public:
        LogManager();
        /**
         * @brief 使用指定名称日志器，若不存在，则创建默认格式的该名日志器
         */
        Logger::ptr getLogger(const std::string &name);

        /**
         * @brief 使用全局日志器
         */
        Logger::ptr getRoot();

        void delLogger(const std::string &name);

        void createOrUpdateLogger(const LoggerDefine &ld);

        Logger::ptr createLogger(const LoggerDefine &ld);

        void updateLogger(Logger::ptr logger, const LoggerDefine &ld);

        LogAppender::ptr getAppender(const std::string &name);

        void delAppender(const std::string &name);

        void createOrUpdateAppender(const LogAppenderDefine &lad);

        LogAppender::ptr createAppender(const LogAppenderDefine &lad);

        void updateAppender(LogAppender::ptr appender, const LogAppenderDefine &lad);

    private:
        void init();

        std::unordered_map<std::string, LogAppender::ptr> m_appenders; // 系统全部输出地
        std::unordered_map<std::string, Logger::ptr> m_loggers;        // 系统全部日志器
        Logger::ptr m_root;                                            // 根日志
        MutexType m_mutex;                                             // 锁
    };

} // namespace lim_webserver
