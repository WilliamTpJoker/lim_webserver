#pragma once

#include <memory>

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
    };
} // namespace lim_webserver
