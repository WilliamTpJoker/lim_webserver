#include "util.h"

namespace lim_webserver
{
    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return 0;
    }
}