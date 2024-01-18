#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <vector>
#include <map>

namespace lim_webserver
{
    class IPAddress;

    class Address
    {
    public:
        using ptr = std::shared_ptr<Address>;

        /**
         * @brief 通过sockaddr指针创建Address
         * @param[in] addr sockaddr指针
         * @param[in] addrlen sockaddr的长度
         * @return 返回和sockaddr相匹配的Address,失败返回nullptr
         */
        static ptr Create(const sockaddr *addr, socklen_t addrlen);

        /**
         * @brief 通过host地址返回对应条件的所有Address
         * @param[out] result 保存满足条件的Address
         * @param[in] host 域名,服务器名等.
         * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
         * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
         * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
         * @return 返回是否转换成功
         */
        static bool Lookup(std::vector<Address::ptr> &result, const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

        /**
         * @brief 通过host地址返回对应条件的任意Address
         * @param[in] host 域名,服务器名等.
         * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
         * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
         * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
         * @return 返回满足条件的任意Address,失败返回nullptr
         */
        static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

        /**
         * @brief 通过host地址返回对应条件的任意IPAddress
         * @param[in] host 域名,服务器名等.
         * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
         * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
         * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
         * @return 返回满足条件的任意IPAddress,失败返回nullptr
         */
        static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

        /**
         * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
         * @param[out] result 保存本机所有地址
         * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
         * @return 是否获取成功
         */
        static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family = AF_INET);

    public:
        virtual ~Address() {}

        /**
         * @brief 获得协议族
         *
         * @return int
         */
        int getFamily() const;

        /**
         * @brief 获得地址 const
         *
         * @return const sockaddr*
         */
        virtual const sockaddr *getAddr() const = 0;

        /**
         * @brief 获得地址
         *
         * @return sockaddr*
         */
        virtual sockaddr *getAddr() = 0;

        /**
         * @brief 获得地址长度
         *
         * @return socklen_t
         */
        virtual socklen_t getAddrLen() const = 0;

        virtual std::string toString() const = 0;

        bool operator<(const Address &rhs) const;
        bool operator==(const Address &rhs) const;
        bool operator!=(const Address &rhs) const;
    };

    class IPAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<IPAddress>;

        /**
         * @brief 通过域名,IP,服务器名创建IPAddress
         * @param[in] address 域名,IP,服务器名等.
         * @param[in] port 端口号
         * @return 调用成功返回IPAddress,失败返回nullptr
         */
        static ptr Create(const char *address, uint16_t port = 0);

    public:
        /**
         * @brief 获得广播地址
         *
         * @param prefix_len
         * @return ptr
         */
        virtual ptr broadcastAddress(uint32_t prefix_len) = 0;

        /**
         * @brief 获得网络地址
         *
         * @param prefix_len
         * @return ptr
         */
        virtual ptr networkAddress(uint32_t prefix_len) = 0;

        /**
         * @brief 获得子网掩码
         *
         * @param prefix_len
         * @return ptr
         */
        virtual ptr subnetMask(uint32_t prefix_len) = 0;

        /**
         * @brief 获得端口号
         *
         * @return uint32_t
         */
        virtual uint32_t getPort() const = 0;

        /**
         * @brief 设置端口号
         *
         * @param port
         */
        virtual void setPort(uint32_t port) = 0;
    };

    class IPv4Address : public IPAddress
    {
    public:
        using ptr = std::shared_ptr<IPv4Address>;

        /**
         * @brief 使用点分十进制地址创建IPv4Address
         * @param[in] address 点分十进制地址,如:192.168.1.1
         * @param[in] port 端口号
         * @return 返回IPv4Address,失败返回nullptr
         */
        static IPv4Address::ptr Create(const char *address, uint16_t port = 0);

    public:
        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
        IPv4Address(const sockaddr_in &address);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;

        std::string toString() const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const override;
        void setPort(uint32_t port) override;

    private:
        sockaddr_in m_addr;
    };

    class IPv6Address : public IPAddress
    {
    public:
        using ptr = std::shared_ptr<IPv6Address>;

        /**
         * @brief 通过IPv6地址字符串构造IPv6Address
         * @param[in] address IPv6地址字符串
         * @param[in] port 端口号
         */
        static IPv6Address::ptr Create(const char *address, uint16_t port = 0);

    public:
        IPv6Address(const uint8_t address[16], uint16_t port = 0);
        IPv6Address(const sockaddr_in6 &address);
        IPv6Address();

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;

        std::string toString() const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const override;
        void setPort(uint32_t port) override;

    private:
        sockaddr_in6 m_addr;
    };

    class UnixAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<UnixAddress>;

    public:
        UnixAddress();
        UnixAddress(const std::string &path);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        void setAddrLen(uint32_t v);

        std::string toString() const override;

    private:
        sockaddr_un m_addr;
        socklen_t m_length;
    };

    class UnknownAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<UnknownAddress>;

    public:
        UnknownAddress(int family);
        UnknownAddress(const sockaddr &addr);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;

        std::string toString() const override;

    private:
        sockaddr m_addr;
    };
}
