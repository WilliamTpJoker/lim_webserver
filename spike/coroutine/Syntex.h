#pragma once

#include "Scheduler.h"

#include <string.h>

namespace lim_webserver
{

    struct Syntex
    {
        Syntex()
        {
        }

        template <typename Function>
        inline void operator-(Function const &f)
        {
            if (!m_scheduler)
                m_scheduler = Processor::GetCurrentScheduler();
            if (!m_scheduler)
                m_scheduler = Scheduler::GetInstance();
            m_scheduler->createTask(f);
        }

        Scheduler *m_scheduler = nullptr;
    };
} // namespace lim_webserver
