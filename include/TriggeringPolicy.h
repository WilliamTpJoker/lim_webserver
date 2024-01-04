#pragma once

#include <memory>
#include "LogSink.h"
#include "LogMessage.h"

namespace lim_webserver
{
    /**
     * 触发策略基类
     */
    class TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<TriggeringPolicy>;

    public:
        /**
         * 校验当前消息体时是否满足了触发条件
         */
        virtual bool isTriggeringMessage(FileSink::ptr file, LogMessage::ptr message) = 0;

    private:
    };

    /**
     * 基于大小的触发策略，当文件大小到达限定时触发
     */
    class SizeBasedTriggeringPolicy : public TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<SizeBasedTriggeringPolicy>;

        const long DEFAULT_MAX_FILE_SIZE = 1024 * 1024 * 1024;

    public:
        bool isTriggeringMessage(FileSink::ptr file, LogMessage::ptr message) override;

        /**
         * 设定最大文件大小
         */
        void setMaxFileSize(long size);

        /**
         * 获取最大文件大小
         */
        const FileSize::ptr &getMaxFileSize();

    private:
        FileSize::ptr m_maxFileSize = FileSize::Create(DEFAULT_MAX_FILE_SIZE); // 最大文件大小
    };

    class TimeBasedFileNameingAndTriggeringPolicy : public TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<TimeBasedFileNameingAndTriggeringPolicy>;

    public:
        long getCurTime();
        void setCurTime(long time);
    };
} // namespace lim_webserver
