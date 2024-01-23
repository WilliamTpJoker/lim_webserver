#include "Server.h"

#include <iostream>

namespace lim_webserver
{
    void TCPServer::bind()
    {
    }

    void TCPServer::start()
    {
        if (m_started)
        {
            return;
        }
        m_started = true;
        for (auto socket : m_socket_vec)
        {
            co[this, socket]
            {
                this->accept(socket);
            };
        }
    }

    void TCPServer::accept(Socket::ptr socket)
    {
        while (m_started)
        {
            Socket::ptr client = socket->accept();
            if (client)
            {
                client->setRecvTimeout(m_recvTimeout);
                co[this, client]
                {
                    this->handleClient(client);
                };
            }
        }
    }

    void TCPServer::handleClient(Socket::ptr client)
    {
        std::cout << "handle" << std::endl;
    }

} // namespace lim_webserver
