#pragma once

#include <memory>
#include <vector>

#include "LogStream.h"
#include "LogMessage.h"
#include "Thread.h"
#include "Fiber.h"

#define LIM_DEFAULT_PATTERN "%d%T%t %N%T%F%T[%c] [%p] %f:%l%T%m%n"

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
