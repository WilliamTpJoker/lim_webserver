#pragma once

#include "Task.h"
#include <atomic>

namespace lim_webserver
{
    class TaskQueue
    {
    public:
        void Enqueue(Task *task);

        bool Dequeue(Task *&task);

        bool Concatenate(TaskQueue& other);

    private:
        std::atomic<Task *> head_{nullptr};
        std::atomic<Task *> tail_{nullptr};
    };
} // namespace lim_webserver
