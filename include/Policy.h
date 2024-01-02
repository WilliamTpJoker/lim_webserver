#pragma once

#include <memory>

#include "LogSink.h"
#include "LogMessage.h"

namespace lim_webserver
{
    class RollingPolicy
    {
    public:
        using ptr = std::shared_ptr<RollingPolicy>;

    public:
        virtual bool judge() = 0;

    protected:
    };

    class TimeBasedRollingPolicy : public RollingPolicy
    {
    public:
        using ptr = std::shared_ptr<TimeBasedRollingPolicy>;

    public:
        bool judge() override;

    private:
    };

    class SizeAndTimeBasedRollingPolicy : public RollingPolicy
    {
    public:
        using ptr = std::shared_ptr<SizeAndTimeBasedRollingPolicy>;

    public:
        bool judge() override;

    private:
    };

    class TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<TriggeringPolicy>;

    public:
        virtual bool isTriggeringMessage(FileSink::ptr file, LogMessage::ptr message) = 0;

    private:
    };

    class SizeBasedTriggeringPolicy : public TriggeringPolicy
    {
    public:
        using ptr = std::shared_ptr<SizeBasedTriggeringPolicy>;

    public:
        bool isTriggeringMessage(FileSink::ptr file, LogMessage::ptr message) override;
    };

} // namespace lim_webserver
