#ifndef _UTIL_H_
#define _UTIL_H_

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <memory>

namespace lim_webserver
{
    // 类智能指针
    template <class T>
    using Shared_ptr = std::shared_ptr<T>;
    // 创建智能指针类
    template <typename T, typename... Args>
    std::shared_ptr<T> MakeShared(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    pid_t GetThreadId();
    uint32_t GetFiberId();
}

#endif