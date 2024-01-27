#include "Server.h"
#include "splog.h"
#include "base/Configer.h"

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    static ConfigerVar<uint64_t>::ptr g_tcp_server_read_timeout = Configer::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
                                                                                   "tcp server read timeout");

    TcpServer::TcpServer(const std::string &name)
        : Server(name), m_recvTimeout(g_tcp_server_read_timeout->getValue())
    {
    }

    TcpServer::~TcpServer()
    {
        stop();
    }

    bool TcpServer::bind(Address::ptr addr, bool ssl)
    {
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> fails;
        addrs.push_back(addr);
        m_ssl = ssl;
        for (auto &addr : addrs)
        {
            Socket::ptr sock = Socket::CreateTCP(addr);
            if (!sock->bind(addr))
            {
                LOG_ERROR(g_logger) << "bind fail errno="
                                    << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            if (!sock->listen())
            {
                LOG_ERROR(g_logger) << "listen fail errno="
                                    << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            m_socket_vec.push_back(sock);
        }

        if (!fails.empty())
        {
            m_socket_vec.clear();
            return false;
        }

        for (auto &socket : m_socket_vec)
        {
            LOG_INFO(g_logger) << " name=" << m_name
                               << " ssl=" << m_ssl
                               << " server bind success: " << socket->peerAddress()->getAddr();
        }
        return true;
    }

    void TcpServer::start()
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

    void TcpServer::stop()
    {
        if (!m_started)
        {
            return;
        }
        m_started = false;
        for (auto &socket : m_socket_vec)
        {
            socket->close();
        }
        m_socket_vec.clear();
    }

    void TcpServer::accept(Socket::ptr socket)
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

    void TcpServer::handleClient(Socket::ptr client)
    {
        LOG_INFO(g_logger) << "TcpServer handle function for client(ip = " << client->peerAddress()->getAddr() << ")";
    }

} // namespace lim_webserver
