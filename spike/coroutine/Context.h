#pragma once

#include <functional>
#include <memory>
#include <ucontext.h>

#include <boost/context/all.hpp>

#include "base/Allocator.h"

namespace lim_webserver
{

    using ContextFunc = void (*)();

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
        Context(ContextFunc func, uintptr_t ptr, size_t stacksize);

        ~Context();

        inline void swapIn() { swapcontext(&getTlsContext(), &m_context); }

        inline void swapOut() { swapcontext(&m_context, &getTlsContext()); }

    private:
        void *m_stack = nullptr; // 栈
        ucontext_t m_context;    // 上下文
        size_t m_stacksize;      // 栈大小
    };

    // class BoostContext
    // {
    //     using context_t = ::boost::context::;

    // public:
    //     Context(std::size_t stack_size, std::function<void()> const &fn)
    //         : protect_page_(0), ctx_(std::allocator_arg, my_fixedsize_stack_allocator(protect_page_, stack_size),
    //                                  [=](context_t yield) mutable -> context_t
    //                                  {
    //                                      this->yield_ = &yield;
    //                                      fn();
    //                                      return std::move(*this->yield_);
    //                                  }),
    //           yield_(nullptr)
    //     {
    //     }

    //     inline bool swapIn()
    //     {
    //         ctx_ = ctx_();
    //         return true;
    //     }

    //     inline bool swapOut()
    //     {
    //         *yield_ = (*yield_)();
    //         return true;
    //     }

    // private:
    //     uint32_t protect_page_;
    //     context_t ctx_;
    //     context_t *yield_;
    // };
} // namespace lim_webserver
