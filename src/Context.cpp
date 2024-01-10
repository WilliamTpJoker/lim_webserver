#include "Context.h"
#include "Config.h"

namespace lim_webserver
{
    static ConfigVar<uint32_t>::ptr s_Task_stack_size = Config::Lookup<uint32_t>("Task.stack_size", 128 * 1024, "Task stack size");

    Context::Context(size_t stacksize)
        : m_stacksize(stacksize)
    {
        m_stacksize = stacksize ? stacksize : s_Task_stack_size->getValue();

        // 分配协程栈内存
        m_stack = Allocator::Alloc(m_stacksize);

        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stacksize;
    }
    Context::~Context()
    {
        if(m_stack)
        {
            Allocator::Dealloc(m_stack, m_stacksize);
        }
    }
} // namespace lim_webserver
