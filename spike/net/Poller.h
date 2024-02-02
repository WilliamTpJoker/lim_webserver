#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "net/IoChannel.h"

#include <memory>
#include <vector>
#include <map>

struct epoll_event;

namespace lim_webserver
{
    class EventLoop;

    class Poller : public Noncopyable
    {
    public:
        using ptr = std::unique_ptr<Poller>;
        using MutexType = Spinlock;

    public:
        Poller();
        virtual ~Poller() {}

        virtual void updateChannel(IoChannel::ptr channel)=0;
        
        virtual void removeChannel(IoChannel::ptr channel)=0;

        /**
         * @brief 读取io
         *
         * @param ms
         */
        virtual void poll(int ms) = 0;

    protected:
        std::map<int, IoChannel::ptr> m_channel_map; // 管理的所有channel
        MutexType m_mutex;
    };

    class EpollPoller : public Poller
    {
    public:
        using ptr = std::unique_ptr<EpollPoller>;

    public:
        EpollPoller();
        ~EpollPoller();

        void updateChannel(IoChannel::ptr channel) override;
        
        void removeChannel(IoChannel::ptr channel) override;

        /**
         * @brief 读取io
         *
         * @param ms
         */
        void poll(int ms) override;

    private:
        /**
         * @brief 修改Fd的行为
         *
         * @param op
         * @param channel
         */
        void update(int op, IoChannel::ptr channel);

        static const char *opToString(int op);

    private:
        int m_epfd;                           // 文件句柄
        std::vector<epoll_event> m_event_vec; // epoll事件vec
    };
} // namespace lim_webserver
