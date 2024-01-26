#pragma once

#include <string>

#include "coroutine.h"
#include "net/Socket.h"
#include "net/EventLoop.h"

#include <vector>

namespace lim_webserver
{
    class Server
    {
    public:
        Server(const std::string &name) : m_name(name) {}
        ~Server() {}

        virtual void start() = 0;

        virtual void stop() = 0;

    protected:
        EventLoop *m_eventloop;
        bool m_started = false;
        std::string m_name;
    };

    class TcpServer : public Server
    {
    public:
        TcpServer(const std::string &name);
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
    };

} // namespace lim_webserver
