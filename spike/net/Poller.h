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
        Poller(EventLoop *loop);
        virtual ~Poller() {}

        IoChannel *getChannel(int fd);

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
         * @brief 读取io
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

        /**
         * @brief 更新channel
         *
         * @param channel
         */
        void updateChannel(IoChannel *channel) override;

        /**
         * @brief 移除channel
         *
         * @param channel
         */
        void removeChannel(IoChannel *channel) override;

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
        void update(int op, IoChannel *channel);

        static const char* opToString(int op);

    private:
        int m_epfd;                           // 文件句柄
        std::vector<epoll_event> m_event_vec; // epoll事件vec
    };
} // namespace lim_webserver
