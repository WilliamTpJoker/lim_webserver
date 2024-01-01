#include "Policy.h"

namespace lim_webserver
{
    bool TimeBasedRollingPolicy::judge()
    {
        return false;
    }

    bool SizeAndTimeBasedRollingPolicy::judge()
    {
        return false;
    }

} // namespace lim_webserver
