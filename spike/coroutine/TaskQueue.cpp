#include "TaskQueue.h"

namespace lim_webserver
{
    void TaskQueue::Enqueue(Task *task)
    {
        Task *prevTail = tail_.exchange(task, std::memory_order_acq_rel);

        if (prevTail != nullptr)
        {
            prevTail->m_next = task;
        }
        else
        {
            // If the queue was empty, update head as well
            head_.store(task, std::memory_order_release);
        }
    }

    bool TaskQueue::Dequeue(Task *&task)
    {
        Task *currentHead = head_.load(std::memory_order_acquire);

        if (currentHead == nullptr)
        {
            // Queue is empty
            return false;
        }

        Task *newHead = currentHead->m_next;

        task = std::move(currentHead);
        if (newHead != nullptr)
        {
            head_.store(newHead, std::memory_order_release);
            return true;
        }

        // The last element is being dequeued
        head_.store(nullptr, std::memory_order_release);
        tail_.store(nullptr, std::memory_order_release);
        return true;
    }

    bool TaskQueue::Concatenate(TaskQueue &other)
    {
        Task *otherHead = other.head_.exchange(nullptr, std::memory_order_acq_rel);

        if (otherHead != nullptr)
        {
            Task *prevTail = tail_.exchange(otherHead, std::memory_order_acq_rel);

            if (prevTail != nullptr)
            {
                prevTail->m_next = otherHead;
            }
            else
            {
                head_.store(otherHead, std::memory_order_release);
            }

            // Update tail to the tail of the other queue
            tail_.store(other.tail_.exchange(nullptr, std::memory_order_acquire), std::memory_order_release);
            return true;
        }
        else
        {
            return false;
        }
    }

} // namespace lim_webserver
