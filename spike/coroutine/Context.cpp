#include "coroutine/Context.h"
#include "base/Configer.h"

namespace lim_webserver
{
    static ConfigerVar<uint32_t>::ptr s_Task_stack_size = Configer::Lookup<uint32_t>("Task.stack_size", 128 * 1024, "Task stack size");

    Context::Context(ContextFunc func, uintptr_t ptr, size_t stacksize)
        : m_stacksize(stacksize)
    {
        m_stacksize = stacksize ? stacksize : s_Task_stack_size->getValue();
        // 分配协程栈内存
        m_stack = Allocator::Alloc(m_stacksize);

        getcontext(&m_context);
        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stacksize;

        makecontext(&m_context, func, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
    }
    Context::~Context()
    {
        if(m_stack)
        {
            Allocator::Dealloc(m_stack, m_stacksize);
        }
    }
} // namespace lim_webserver
