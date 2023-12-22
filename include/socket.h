#pragma once

#include <memory>
#include <netinet/tcp.h>

#include "address.h"
#include "Noncopyable.h"

namespace lim_webserver
{
    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable
    {
    public:
        using ptr = std::shared_ptr<Socket>;
        using weak_ptr = std::weak_ptr<Socket>;

        /**
         * @brief Socket类型
         */
        enum Type
        {
            /// TCP类型
            TCP = SOCK_STREAM,
            /// UDP类型
            UDP = SOCK_DGRAM
        };

        /**
         * @brief Socket协议簇
         */
        enum Family
        {
            /// IPv4 socket
            IPv4 = AF_INET,
            /// IPv6 socket
            IPv6 = AF_INET6,
            /// Unix socket
            UNIX = AF_UNIX,
        };

        static Socket::ptr CreateTCP(Address::ptr address);
        static Socket::ptr CreateUDP(Address::ptr address);

        /**
         * @brief 创建IPv4的TCP Socket
         */
        static Socket::ptr CreateTCPSocket();

        /**
         * @brief 创建IPv4的UDP Socket
         */
        static Socket::ptr CreateUDPSocket();

        /**
         * @brief 创建IPv6的TCP Socket
         */
        static Socket::ptr CreateTCPSocket6();

        /**
         * @brief 创建IPv6的UDP Socket
         */
        static Socket::ptr CreateUDPSocket6();

        /**
         * @brief 创建Unix的TCP Socket
         */
        static Socket::ptr CreateUnixTCPSocket();

        /**
         * @brief 创建Unix的UDP Socket
         */
        static Socket::ptr CreateUnixUDPSocket();

    public:
        Socket(int family, int type, int protocol = 0);
        ~Socket();

        int64_t getSendTimeout();
        void setSendTimeout(int64_t v);

        int64_t getRecvTimeout();
        void setRecvTimeout(int64_t v);

        bool getOption(int level, int option, void *result, socklen_t *len);
        template <class T>
        bool getOption(int level, int option, T &result)
        {
            return getOption(level, option, &result, sizeof(T));
        }

        bool setOption(int level, int option, const void *value, socklen_t len);

        template <class T>
        bool setOption(int level, int option, const T &value)
        {
            return setOption(level, option, &value, sizeof(T));
        }

        Socket::ptr accept();
        bool bind(const Address::ptr addr);
        bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
        bool listen(int backlog = SOMAXCONN);
        bool close();

        int send(const void *buffer, size_t length, int flags = 0);
        int send(const iovec *buffers, size_t length, int flags = 0);
        int sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0);
        int sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags = 0);

        int recv(void *buffer, size_t length, int flags = 0);
        int recv(iovec *buffers, size_t length, int flags = 0);
        int recvFrom(void *buffer, size_t length, const Address::ptr from, int flags = 0);
        int recvFrom(iovec *buffers, size_t length, const Address::ptr from, int flags = 0);

        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();

        int getFamily() const { return m_family; }
        int getType() const { return m_type; }
        int getProtocol() const { return m_protocol; }

        bool isConnected() const { return m_isConnected; }
        bool isValid() const;
        int getError();

        std::ostream &dump(std::ostream &os) const;
        int getSocket() const { return m_sock; }

        bool cancelRead();
        bool cancelWrite();
        bool cancelAccept();
        bool cancelAll();

    private:
        void initSock();
        void newSock();
        bool init(int sock);

    private:
        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConnected;

        Address::ptr m_localAddress;
        Address::ptr m_remoteAddress;
    };
} // namespace lim_webserver
