#ifndef __LIM_HOOK_H__
#define __LIM_HOOK_H__

#include <unistd.h>

namespace lim_webserver
{
    bool is_hook_enable();
    void set_hook_enable(bool flag);
}

extern "C"
{
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef int (*usleep_fun)(useconds_t usec);
    extern usleep_fun usleep_f;
}
#endif