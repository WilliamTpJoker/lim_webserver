#pragma once

#include "Socket.h"
#include "IoChannel.h"

namespace lim_webserver
{
    class EventLoop;

    class Accepter
    {
    public:
        Accepter(EventLoop* loop, Address::ptr address);

        void listen();

    private:
        void HandleRead();

    private:
        EventLoop *m_loop;
        bool m_listening = false; // 监听标志位
        Socket::ptr m_socket;
        IoChannel m_channel;
    };
} // namespace lim_webserver
