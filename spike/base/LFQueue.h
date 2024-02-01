#include <atomic>

namespace lim_webserver
{

    template <typename T> class LFQueue
    {
    public:
        LFQueue() {}

        ~LFQueue()
        {
            Node *current = head_.load();
            while (current)
            {
                Node *to_delete = current;
                current = current->next_;
                delete to_delete;
            }
        }

        void enqueue(const T &value)
        {
            Node *new_node = new Node(value);
            Node *old_tail = tail_.exchange(new_node, std::memory_order_acq_rel);
            if (old_tail)
            {
                old_tail->next_ = new_node;
            }
            else
            {
                head_.store(new_node);
            }
        }

        bool dequeue(T &value)
        {
            Node *old_head = head_.load(std::memory_order_relaxed);
            while (old_head && !head_.compare_exchange_weak(old_head, old_head->next_, std::memory_order_acquire, std::memory_order_relaxed))
            {
                // CAS失败时，继续尝试
            }
            if (old_head)
            {
                value = old_head->data_;
                delete old_head;
                return true;
            }
            tail_.exchange(nullptr);
            return false;
        }

        bool concatenate(LFQueue<T> &other)
        {
            Node *otherHead = other.head_.exchange(nullptr, std::memory_order_acq_rel);

            if (otherHead != nullptr)
            {
                Node *prevTail = tail_.exchange(otherHead, std::memory_order_acq_rel);

                // 有尾则接上
                if (prevTail != nullptr)
                {
                    prevTail->next_ = otherHead;
                }
                else // 无尾则为新的
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

    private:
        struct Node
        {
            T data_;
            Node *next_ = nullptr;

            Node(const T &data) : data_(data) {}
        };

        std::atomic<Node *> head_{nullptr};
        std::atomic<Node *> tail_{nullptr};
    };

} // namespace lim_webserver