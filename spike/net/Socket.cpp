#include "net/Socket.h"
#include "splog/splog.h"

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_NAME("system");

    Socket::ptr Socket::CreateTCP(Address::ptr address)
    {
        Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDP(Address::ptr address)
    {
        Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateTCPSocket()
    {
        Socket::ptr sock(new Socket(IPv4, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDPSocket()
    {
        Socket::ptr sock(new Socket(IPv4, UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateTCPSocket6()
    {
        Socket::ptr sock(new Socket(IPv6, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDPSocket6()
    {
        Socket::ptr sock(new Socket(IPv6, UDP, 0));
        sock->newSock();
        sock->m_isConnected = true;
        return sock;
    }

    Socket::ptr Socket::CreateUnixTCPSocket()
    {
        Socket::ptr sock(new Socket(UNIX, TCP, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUnixUDPSocket()
    {
        Socket::ptr sock(new Socket(UNIX, UDP, 0));
        return sock;
    }

    Socket::Socket(int family, int type, int protocol)
        : m_fd(-1), m_family(family), m_type(type), m_protocol(protocol), m_isConnected(false)
    {
    }

    Socket::~Socket()
    {
        close();
    }

    // int64_t Socket::getSendTimeout()
    // {
    //     FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_fd);
    //     if (ctx)
    //     {
    //         return ctx->getTimeout(SO_SNDTIMEO);
    //     }
    //     return -1;
    // }

    void Socket::setSendTimeout(int64_t v)
    {
        struct timeval tv
        {
            int(v / 1000), int(v % 1000 * 1000)
        };
        setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    }

    // int64_t Socket::getRecvTimeout()
    // {
    //     FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_fd);
    //     if (ctx)
    //     {
    //         return ctx->getTimeout(SO_RCVTIMEO);
    //     }
    //     return -1;
    // }

    void Socket::setRecvTimeout(int64_t v)
    {
        struct timeval tv
        {
            int(v / 1000), int(v % 1000 * 1000)
        };
        setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }

    bool Socket::getOption(int level, int option, void *result, socklen_t *len)
    {
        int rt = getsockopt(m_fd, level, option, result, len);
        if (rt)
        {
            LOG_DEBUG(g_logger) << "getOption sock=" << m_fd
                                    << " level=" << level << " option=" << option
                                    << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void *value, socklen_t len)
    {
        if (setsockopt(m_fd, level, option, value, len))
        {
            LOG_DEBUG(g_logger) << "setOption sock=" << m_fd
                                    << " level=" << level << " option=" << option
                                    << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    Socket::ptr Socket::accept()
    {
        Socket::ptr sock = std::make_shared<Socket>(m_family, m_type, m_protocol);
        int newsock = ::accept(m_fd, nullptr, nullptr);
        if (newsock == -1)
        {
            LOG_ERROR(g_logger) << "accept(" << m_fd << ") errno="
                                    << errno << " errstr=" << strerror(errno);
            return nullptr;
        }
        if (sock->init(newsock))
        {
            return sock;
        }
        return nullptr;
    }

    // bool Socket::init(int sock)
    // {
    //     FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    //     if (ctx && ctx->isSocket() && !ctx->isClosed())
    //     {
    //         m_fd = sock;
    //         m_isConnected = true;
    //         initSock();
    //         getLocalAddress();
    //         getRemoteAddress();
    //         return true;
    //     }
    //     return false;
    // }

    bool Socket::bind(const Address::ptr addr)
    {
        if (!isValid())
        {
            newSock();
            if (UNLIKELY(!isValid()))
            {
                return false;
            }
        }
        if (UNLIKELY(addr->getFamily() != m_family))
        {
            LOG_ERROR(g_logger) << "bind sock.family("
                                    << m_family << ") addr.family(" << addr->getFamily()
                                    << ") not equal, addr=" << addr->toString();
            return false;
        }

        if (::bind(m_fd, addr->getAddr(), addr->getAddrLen()))
        {
            LOG_ERROR(g_logger) << "bind error errrno=" << errno
                                    << " errstr=" << strerror(errno);
            return false;
        }
        getLocalAddress();
        return true;
    }

    bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms)
    {
        if (!isValid())
        {
            newSock();
            if (UNLIKELY(!isValid()))
            {
                return false;
            }
        }

        if (UNLIKELY(addr->getFamily() != m_family))
        {
            LOG_ERROR(g_logger) << "connect sock.family("
                                    << m_family << ") addr.family(" << addr->getFamily()
                                    << ") not equal, addr=" << addr->toString();
            return false;
        }

        if (timeout_ms == (uint64_t)-1)
        {
            if (::connect(m_fd, addr->getAddr(), addr->getAddrLen()))
            {
                LOG_ERROR(g_logger) << "sock=" << m_fd << " connect(" << addr->toString()
                                        << ") error errno=" << errno << " errstr=" << strerror(errno);
                close();
                return false;
            }
        }
        m_isConnected = true;
        getRemoteAddress();
        getLocalAddress();
        return true;
    }

    bool Socket::listen(int backlog)
    {
        if (!isValid())
        {
            LOG_ERROR(g_logger) << "listen error sock=-1";
            return false;
        }
        if (::listen(m_fd, backlog) == 0)
        {
            LOG_ERROR(g_logger) << "listen error errno=" << errno
                                    << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::close()
    {
        if (!m_isConnected && m_fd == -1)
        {
            return true;
        }
        m_isConnected = false;
        if (m_fd != -1)
        {
            ::close(m_fd);
            m_fd = -1;
        }
        return false;
    }

    int Socket::send(const void *buffer, size_t length, int flags)
    {
        if (isConnected())
        {
            return ::send(m_fd, buffer, length, flags);
        }
        return -1;
    }

    int Socket::send(const iovec *buffers, size_t length, int flags)
    {
        if (isConnected())
        {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)buffers;
            msg.msg_iovlen = length;
            return ::sendmsg(m_fd, &msg, flags);
        }
        return -1;
    }

    int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags)
    {
        if (isConnected())
        {
            return ::sendto(m_fd, buffer, length, flags, to->getAddr(), to->getAddrLen());
        }
        return -1;
    }

    int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags)
    {
        if (isConnected())
        {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(m_fd, &msg, flags);
        }
        return -1;
    }

    int Socket::recv(void *buffer, size_t length, int flags)
    {
        if (isConnected())
        {
            return ::recv(m_fd, buffer, length, flags);
        }
        return -1;
    }

    int Socket::recv(iovec *buffers, size_t length, int flags)
    {
        if (isConnected())
        {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)buffers;
            msg.msg_iovlen = length;
            return ::recvmsg(m_fd, &msg, flags);
        }
        return -1;
    }

    int Socket::recvFrom(void *buffer, size_t length, const Address::ptr from, int flags)
    {
        if (isConnected())
        {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(m_fd, buffer, length, flags, from->getAddr(), &len);
        }
        return -1;
    }

    int Socket::recvFrom(iovec *buffers, size_t length, const Address::ptr from, int flags)
    {
        if (isConnected())
        {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(m_fd, &msg, flags);
        }
        return -1;
    }

    Address::ptr Socket::getRemoteAddress()
    {
        if (m_remoteAddress)
        {
            return m_remoteAddress;
        }

        Address::ptr result;
        switch (m_family)
        {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
        }
        socklen_t addrlen = result->getAddrLen();
        if (getpeername(m_fd, result->getAddr(), &addrlen))
        {
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_remoteAddress = result;
        return m_remoteAddress;
    }

    Address::ptr Socket::getLocalAddress()
    {
        if (m_localAddress)
        {
            return m_localAddress;
        }

        Address::ptr result;
        switch (m_family)
        {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
        }
        socklen_t addrlen = result->getAddrLen();
        if (getsockname(m_fd, result->getAddr(), &addrlen))
        {
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_localAddress = result;
        return m_localAddress;
    }

    bool Socket::isValid() const
    {
        return m_fd != -1;
    }

    int Socket::getError()
    {
        int error = 0;
        socklen_t len = sizeof(error);
        if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len))
        {
            error = errno;
        }
        return error;
    }

    void Socket::initSock()
    {
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if (m_type == TCP)
        {
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }

    void Socket::newSock()
    {
        m_fd = socket(m_family, m_type, m_protocol);
        if (LIKELY(m_fd != -1))
        {
            initSock();
        }
        else
        {
            LOG_ERROR(g_logger) << "socket(" << m_family
                                    << ", " << m_type << ", " << m_protocol << ") errno="
                                    << errno << " errstr=" << strerror(errno);
        }
    }

} // namespace lim_webserver
