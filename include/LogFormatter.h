#pragma once

#include <memory>
#include <vector>

#include "LogStream.h"
#include "LogMessage.h"
#include "Thread.h"

/**
 * %p 输出日志等级
 * %f 输出文件名
 * %l 输出行号
 * %d 输出日志时间
 * %t 输出线程号
 * %N 输出线程名
 * %C 输出协程号
 * %m 输出日志消息
 * %n 输出换行
 * %% 输出百分号
 * %T 输出制表符
 * %r 输出自启动到现在的时间
 * %c 输出日志信息所属的类目
 * */
#define DEFAULT_PATTERN "%d%T%t %N%T%C%T[%c] [%p] %f:%l%T%m%n"

namespace lim_webserver
{
    /**
     * @brief 日志格式器
     */
    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        static ptr Create(const std::string &pattern)
        {
            return std::make_shared<LogFormatter>(pattern);
        }

    public:
        LogFormatter(const std::string &pattern);

        /**
         * @brief 构造字符流
         */
        void format(LogStream &stream, LogMessage::ptr event);
        /**
         * @brief 获得格式
         */
        const std::string &getPattern() const { return m_pattern; }
        /**
         * @brief 判断格式是否异常
         * @return true：异常 false: 正常
         */
        bool isError() { return m_error; }

        /**
         * @brief 格式器初始化
         */
        void init();

    public:
        /**
         * @brief 格式体虚父类
         */
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}

            /**
             * @brief 构造字符流
             */
            virtual void format(LogStream &stream, LogMessage::ptr event) = 0;
        };

    private:
        std::string m_pattern;                            // 格式
        std::vector<std::shared_ptr<FormatItem>> m_items; // 格式体容器
        bool m_error = false;                             // 异常标志符
    };

} // namespace lim_webserver
