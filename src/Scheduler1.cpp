#include "Scheduler1.h"

namespace lim_webserver
{
    void Scheduler1::start(int num_threads)
    {
        if (m_started)
        {
            return;
        }
        m_threadCounts = num_threads;

        // 创建主处理器
        m_mainProcessor = new Processor(this, 0);
        m_processors.push_back(m_mainProcessor);

        // 创建从处理器
        for (int i = 0; i < num_threads - 1; ++i)
        {
            newPoccessorThread();
        }
        m_started = true;

        // 创建调度线程
        m_thread = Thread::Create([this]()
                                  { this->run(); },
                                  "Scheduler");

        // 开始处理器                          
        m_mainProcessor->start();
    }

    void Scheduler1::stop()
    {
        if (!m_started)
        {
            return;
        }
        m_started = false;
        // TODO: 关闭所有处理器
        // TODO: 关闭调度线程
    }

    Scheduler1::~Scheduler1()
    {
        stop();
    }

    void Scheduler1::run()
    {
        while (m_started)
        {
            
        }
    }

    void Scheduler1::newPoccessorThread()
    {
        Processor *p = new Processor(this, m_processors.size());
        p->start();
        m_processors.push_back(p);
    }

} // namespace lim_webserver
