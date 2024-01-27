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
        Poller();
        virtual ~Poller() {}

        /**
         * @brief 添加event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        virtual bool addEvent(int fd, IoEvent event);

        /**
         * @brief 取消event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        virtual bool cancelEvent(int fd, IoEvent event);

        /**
         * @brief 清空event
         *
         * @param fd
         * @return true
         * @return false
         */
        virtual bool clearEvent(int fd);

        /**
         * @brief 读取io
         *
         * @param ms
         */
        virtual void poll(int ms) = 0;

    protected:
        void channelVecResize(size_t size);

    protected:
        std::vector<IoChannel *> m_channel_vec; // 管理的所有channel
    };

    class EpollPoller : public Poller
    {
    public:
        using ptr = std::unique_ptr<EpollPoller>;

    public:
        EpollPoller();
        ~EpollPoller();

        /**
         * @brief 添加event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        bool addEvent(int fd, IoEvent event) override;

        /**
         * @brief 取消event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        bool cancelEvent(int fd, IoEvent event) override;

        /**
         * @brief 清空event
         *
         * @param fd
         * @return true
         * @return false
         */
        bool clearEvent(int fd) override;

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

        static const char *opToString(int op);

    private:
        int m_epfd;                           // 文件句柄
        std::vector<epoll_event> m_event_vec; // epoll事件vec
    };
} // namespace lim_webserver
