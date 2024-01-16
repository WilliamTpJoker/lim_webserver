#pragma once

#include "FdContext.h"
#include "Noncopyable.h"

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
        virtual ~Poller() = 0;

        /**
         * @brief 更新Context
         *
         * @param context
         */
        virtual void updateContext(FdContext *context) = 0;

        /**
         * @brief 移除Context
         *
         * @param context
         */
        virtual void removeContext(FdContext *context) = 0;

        /**
         * @brief 确认是否存在Context
         *
         * @param context
         * @return true
         * @return false
         */
        bool hasContext(FdContext *context);

        /**
         * @brief 获取响应时间
         *
         * @param ms
         */
        virtual void poll(int ms) = 0;

    protected:
        std::map<int, FdContext *> m_contexts_map; // 管理的所有context

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

        void updateContext(FdContext *context) override;
        void removeContext(FdContext *context) override;
        void poll(int ms) override;

    private:
        /**
         * @brief 修改Fd的行为
         *
         * @param op
         * @param context
         */
        void update(int op, FdContext *context);

    private:
        int m_epfd;                           // 文件句柄
        std::vector<epoll_event> m_event_vec; // epoll事件vec
    };
} // namespace lim_webserver
