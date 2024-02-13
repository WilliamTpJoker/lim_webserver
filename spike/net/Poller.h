#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "net/IoChannel.h"

#include <map>
#include <memory>
#include <vector>

struct epoll_event;

namespace lim_webserver
{
    class EventLoop;

    class Poller : public Noncopyable
    {
    public:
        using ptr = std::unique_ptr<Poller>;
        using MutexType = Spinlock;
        using ChannelMap = std::map<int, IoChannel::ptr>;

    public:
        Poller();
        virtual ~Poller() {}

        virtual void updateChannel(IoChannel::ptr channel) = 0;

        virtual void removeChannel(IoChannel::ptr channel) = 0;

        bool hasChannel(IoChannel::ptr channel) const;

        /**
         * @brief 读取io
         *
         * @param ms
         */
        virtual void poll(int ms) = 0;

        virtual void bindEventFd(int fd) = 0;

    protected:
        int m_wakefd;             // 唤醒句柄
        ChannelMap m_channel_map; // 管理的所有channel
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

        void bindEventFd(int fd) override;

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
