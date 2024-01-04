#include "RollingPolicy.h"
#include "LogAppender.h"

namespace lim_webserver
{
    void RollingPolicy::setFileNamePattern(const std::string &pattern)
    {
        m_pattern = pattern;
    }

    const std::string &RollingPolicy::getFileNamePattern()
    {
        return m_pattern;
    }

    void RollingPolicy::setParent(std::shared_ptr<FileAppender> appender)
    {
        m_parent = appender;
    }

    const std::string &RollingPolicy::getParentsRawFileProperty()
    {
        return m_parent->rawFileProperty();
    }

    void TimeBasedRollingPolicy::rollover()
    {
    }

} // namespace lim_webserver
