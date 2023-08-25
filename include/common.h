#ifndef __LIM_COMMON_H__
#define __LIM_COMMON_H__

#include <memory>

namespace lim_webserver
{
    // 类智能指针
    template <class T>
    using Shared_ptr = std::shared_ptr<T>;
    
    template <class T>
    using Unique_ptr = std::unique_ptr<T>;

    // 创建智能指针类
    template <typename T, typename... Args>
    Shared_ptr<T> MakeShared(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

#endif