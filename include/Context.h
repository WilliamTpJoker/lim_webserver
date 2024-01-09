#pragma once

#include <ucontext.h>
#include <functional>

#include "Allocator.h"

namespace lim_webserver
{
    using FuncType = std::function<void()>;

    class Context
    {
    public:
        /**
         * @brief 获得线程局部存储(Thread local storage)上下文
         *
         * @return ucontext_t 上下文
         */
        ucontext_t &getTlsContext()
        {
            static thread_local ucontext_t tls_context;
            return tls_context;
        }

    public:
        Context(FuncType callback, size_t stacksize);

        ~Context();

        inline void swapIn() { swapcontext(&getTlsContext(), &m_context); }

        inline void swapTo(Context &other) { swapcontext(&other.m_context, &m_context); }

        inline void swapOut() { swapcontext(&m_context, &getTlsContext()); }

    private:
        void *m_stack = nullptr; // 栈
        ucontext_t m_context;    // 上下文
        size_t m_stacksize;      // 栈大小
        FuncType m_callback;     // 回调函数
    };
} // namespace lim_webserver
