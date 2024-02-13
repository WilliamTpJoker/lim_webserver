#pragma once

#include <string>

#include "coroutine.h"
#include "net/EventLoop.h"
#include "net/Socket.h"

#include <vector>

namespace lim_webserver
{
    class Server
    {
    public:
        Server() {}
        ~Server() {}

        inline void setName(const std::string &name) { m_name = name; }
        inline const std::string &name() const { return m_name; }

        virtual void start() = 0;

        virtual void stop() = 0;

    protected:
        bool m_started = false;
        std::string m_name = "default_server";
    };

    class TcpServer : public Server
    {
    public:
        TcpServer(Scheduler *worker = EventLoop::GetCurrentScheduler(), Scheduler *accepter = EventLoop::GetCurrentScheduler());
        ~TcpServer();

        bool bind(Address::ptr addr, bool ssl = false);

        void start() override;

        void stop() override;

    protected:
        void accept(Socket::ptr socket);

        /**
         * @brief 统一的回调
         *
         * @param client
         */
        virtual void handleClient(Socket::ptr client);

    protected:
        std::vector<Socket::ptr> m_socket_vec;
        uint64_t m_recvTimeout;
        bool m_ssl;
        Scheduler *m_worker;
        Scheduler *m_accepter;
    };

} // namespace lim_webserver
