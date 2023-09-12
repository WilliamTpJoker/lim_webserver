#include <dlfcn.h>

#include "hook.h"
#include "fiber.h"
#include "thread.h"
#include "io_manager.h"

namespace lim_webserver
{
    static thread_local bool t_hook_enable = false;

#define HOOK_FUN(F) \
    F(sleep)        \
    F(usleep)

    void hook_init()
    {
        static bool is_inited = false;
        if (is_inited)
        {
            return;
        }
#define LOAD_HOOK_FUN(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(LOAD_HOOK_FUN);
#undef LOAD_HOOK_FUN
    }

    struct  _HookIniter
    {
        _HookIniter()
        {
            hook_init();
        }
    };
    
    static _HookIniter s_hookIniter;

    bool is_hook_enable()
    {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag)
    {
        t_hook_enable = flag;
    }

    extern "C"
    {
#define DEF_FUN_NAME(name) name##_fun name##_f = nullptr;
        // 定义系统 api 的函数指针的变量
        HOOK_FUN(DEF_FUN_NAME);
#undef DEF_FUN_NAME

        unsigned int sleep(unsigned int seconds)
        {
            if(!t_hook_enable)
            {
                return sleep_f(seconds);
            }
            
            Fiber::ptr fiber = Fiber::GetThis();
            IoManager* iom = IoManager::GetThis();
            iom->addTimer(seconds*1000,[iom, fiber](){ iom->schedule(fiber, -1); });
            Fiber::YieldToHold();
            return 0;
        }

        int usleep(useconds_t usec)
        {
            if(!t_hook_enable)
            {
                return usleep_f(usec);
            }

            Fiber::ptr fiber = Fiber::GetThis();
            IoManager* iom = IoManager::GetThis();
            iom->addTimer(usec/1000,[iom, fiber](){ iom->schedule(fiber, -1); });
            Fiber::YieldToHold();
            return 0; 
        }
    }
}