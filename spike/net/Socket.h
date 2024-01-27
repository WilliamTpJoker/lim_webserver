#pragma once

#include <memory>
#include <netinet/tcp.h>

#include "net/Address.h"
#include "base/Noncopyable.h"

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

        /**
         * @brief 获得端地址
         * 
         * @return Address::ptr 
         */
        Address::ptr peerAddress();

        /**
         * @brief 获得本地地址
         * 
         * @return Address::ptr 
         */
        Address::ptr localAddress();

        /**
         * @brief 获得协议族
         *
         * @return int
         */
        int family() const { return m_family; }

        /**
         * @brief 获得IP类型
         *
         * @return int
         */
        int type() const { return m_type; }

        /**
         * @brief 获得具体协议
         *
         * @return int
         */
        int protocol() const { return m_protocol; }

        /**
         * @brief 确认是否建立连接
         *
         * @return true
         * @return false
         */
        bool isConnected() const { return m_isConnected; }

        /**
         * @brief 获得错误信息
         * 
         * @return int 
         */
        int error();

        /**
         * @brief 获得句柄
         *
         * @return int
         */
        int fd() const { return m_fd; }

    private:
        void initSock();
        bool init(int sock);

    private:
        int m_fd;             // 句柄
        const int m_family;   // 协议族
        const int m_type;     // IP类型
        const int m_protocol; // 具体协议
        bool m_isConnected;   // 连接标志符,TCP需要确认连接

        Address::ptr m_localAddress;
        Address::ptr m_peerAddress;
    };
} // namespace lim_webserver
