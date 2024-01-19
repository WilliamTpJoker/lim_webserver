#pragma once

#include "base/Noncopyable.h"
#include "net/IoChannel.h"

#include <vector>
#include <map>
#include <memory>

struct epoll_event;

namespace lim_webserver
{
    class EventLoop;

    class Poller : public Noncopyable
    {
    public:
        using ptr = std::unique_ptr<Poller>;

    public:
        Poller(EventLoop *loop) : m_loop(loop) {}
        virtual ~Poller(){}

        /**
         * @brief 更新Channel
         *
         * @param channel
         */
        virtual void updateChannel(IoChannel *channel) = 0;

        /**
         * @brief 移除Channel
         *
         * @param channel
         */
        virtual void removeChannel(IoChannel *channel) = 0;

        /**
         * @brief 确认是否存在Channel
         *
         * @param channel
         * @return true
         * @return false
         */
        bool hasChannel(IoChannel *channel);

        /**
         * @brief 获取响应时间
         *
         * @param ms
         */
        virtual void poll(int ms) = 0;

    protected:
        std::map<int, IoChannel *> m_channels_map; // 管理的所有channel

    private:
        EventLoop *m_loop; // 所属的EventLoop
    };

    class EpollPoller : public Poller
    {
    public:
        using ptr = std::unique_ptr<EpollPoller>;

    public:
        EpollPoller(EventLoop *loop);
        ~EpollPoller();

        void updateChannel(IoChannel *channel) override;
        void removeChannel(IoChannel *channel) override;
        void poll(int ms) override;

    private:
        /**
         * @brief 修改Fd的行为
         *
         * @param op
         * @param channel
         */
        void update(int op, IoChannel *channel);

    private:
        int m_epfd;                           // 文件句柄
        std::vector<epoll_event> m_event_vec; // epoll事件vec
    };
} // namespace lim_webserver
