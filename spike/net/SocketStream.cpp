#include "SocketStream.h"

namespace lim_webserver
{
    SocketStream::SocketStream(Socket::ptr sock, bool owner) : m_socket(sock), m_owner(owner) {}

    SocketStream::~SocketStream()
    {
        if (m_owner && m_socket)
        {
            m_socket->close();
        }
    }

    bool SocketStream::isConnected() const { return m_socket && m_socket->isConnected(); }

    int SocketStream::resv(void *buffer, size_t length)
    {
        if (!isConnected())
        {
            return -1;
        }
        return m_socket->recv(buffer, length);
    }

    int SocketStream::resv(ByteArray::ptr ba, size_t length)
    {
        if (isConnected())
        {
            std::vector<iovec> iovs;
            ba->getWriteBuffers(iovs, length);
            int rt = m_socket->recv(&iovs[0], iovs.size());
            if (rt > 0)
            {
                ba->addWritePosition(rt);
            }
            return rt;
        }
        return -1;
    }

    int SocketStream::send(const void *buffer, size_t length)
    {
        if (!isConnected())
        {
            return -1;
        }
        return m_socket->send(buffer, length);
    }

    int SocketStream::send(ByteArray::ptr ba, size_t length)
    {
        if (isConnected())
        {
            std::vector<iovec> iovs;
            ba->getReadBuffers(iovs, length);
            int rt = m_socket->send(&iovs[0], iovs.size());
            if(rt>0)
            {
                ba->addReadPosition(rt);
            }
            return rt;
        }
        return -1;
    }

    void SocketStream::close()
    {
        if (m_socket)
        {
            m_socket->close();
        }
    }

    Address::ptr SocketStream::peerAddress()
    {
        if (m_socket)
        {
            return m_socket->peerAddress();
        }
        return nullptr;
    }

    Address::ptr SocketStream::localAddress()
    {
        if (m_socket)
        {
            return m_socket->localAddress();
        }
        return nullptr;
    }

    std::string SocketStream::peerAddressString()
    {
        auto addr = peerAddress();
        if (addr)
        {
            return addr->toString();
        }
        return "";
    }

    std::string SocketStream::localAddressString()
    {
        auto addr = localAddress();
        if (addr)
        {
            return addr->toString();
        }
        return "";
    }
} // namespace lim_webserver
