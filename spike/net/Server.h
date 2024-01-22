#pragma once

#include <string>

#include "coroutine/coroutine.h"
#include "net/Socket.h"

#include <vector>

namespace lim_webserver
{
    class Server
    {
    public:
        Server(std::string name);
        ~Server() {}

        void start();

        void stop();

    protected:
        Scheduler *m_scheduler;
        bool m_started=false;
    };

    class TCPServer : public Server
    {
        void bind();

        void start();

        void stop();

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
    };

} // namespace lim_webserver
