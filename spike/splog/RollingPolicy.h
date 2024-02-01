#pragma once

#include <memory>

#include "TriggeringPolicy.h"

namespace lim_webserver
{
    class FileAppender;

    /**
     * @brief 滚动策略基类
     */
    class RollingPolicy
    {
    public:
        using ptr = std::shared_ptr<RollingPolicy>;

    public:
        /**
         * @brief 文件滚动
         */
        virtual void rollover() = 0;

        /**
         * @brief 获取当前文件名
         */
        virtual const std::string &getActiveFileName() = 0;

        /**
         * @brief 设置文件命名格式
         */
        void setFileNamePattern(const std::string &pattern);

        /**
         * @brief 获取文件命名格式
         */
        const std::string &getFileNamePattern();

        /**
         * @brief 设置调用该策略的输出地
         */
        void setParent(std::shared_ptr<FileAppender> appender);

        /**
         * @brief 获取输出地的原始文件名
         */
        const std::string &getParentsRawFileProperty();

    protected:
        std::shared_ptr<FileAppender> m_parent; // 调用该策略的输出地
        std::string m_pattern;                  // 文件命名格式
    };

    /**
     * 基于时间的滚动策略，每天一个新文件
     */
    class TimeBasedRollingPolicy : public RollingPolicy, public TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<TimeBasedRollingPolicy>;

    public:
        void rollover() override;

    protected:
    };

    /**
     * 基于时间和大小的滚动策略，每天进行滚动，当文件大小到达限定时滚动
     */
    class SizeAndTimeBasedRollingPolicy : public TimeBasedRollingPolicy
    {
    public:
        using ptr = std::shared_ptr<SizeAndTimeBasedRollingPolicy>;

    public:
    private:
    };

} // namespace lim_webserver
