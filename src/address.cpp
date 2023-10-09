#include "address.h"
#include "log.h"
#include <sstream>
#include <ifaddrs.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LIM_LOG_NAME("system");

    template <class T>
    static T CreateMask(uint32_t bits)
    {
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    template <class T>
    static uint32_t CountBytes(T value)
    {
        uint32_t result = 0;
        for (; value; ++result)
        {
            value &= value - 1;
        }
        return result;
    }

    Address::ptr Address::Create(const sockaddr *addr, socklen_t addrlen)
    {
        if (addr == nullptr)
        {
            return nullptr;
        }

        Address::ptr result;
        switch (addr->sa_family)
        {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in *)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6 *)addr));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
        }
        return result;
    }

    bool Address::Lookup(std::vector<Address::ptr> &result, const std::string &host, int family, int type, int protocol)
    {
        addrinfo hints, *res, *p;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;

        std::string node;
        const char *service = NULL;

        // 如果字符串不为空且以 "[" 开头，表示可能包含 IPv6 地址
        if (!host.empty() && host[0] == '[')
        {
            // 查找 IPv6 地址中的 "]" 字符
            const char *endipv6 = (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
            if (endipv6)
            {
                // 如果 "]" 字符后面跟着一个 ":", 则表示还有端口号
                if (*(endipv6 + 1) == ':')
                {
                    // 将服务名指向 ":" 后的字符
                    service = endipv6 + 2;
                }
                // 提取 IPv6 地址，去除 "[" 和 "]"
                node = host.substr(1, endipv6 - host.c_str() - 1);
            }
        }

        // 检测非ipv6地址
        if (node.empty())
        {
            // 使用 memchr 函数在输入字符串中查找第一个冒号 ':'
            service = (const char *)memchr(host.c_str(), ':', host.size());
            // 如果找到了冒号
            if (service)
            {
                // 进一步检查是否还有一个冒号，如果没有，表示找到了有效的主机名和端口号
                if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1))
                {
                    // 提取主机名部分，存储在变量 node 中
                    node = host.substr(0, service - host.c_str());
                    // 将服务名指向 ":" 后的字符
                    ++service;
                }
            }
        }

        // 如果没查找到冒号，则表示没有端口号
        if (node.empty())
        {
            node = host;
        }

        int error = getaddrinfo(node.c_str(), service, &hints, &res);
        if (error)
        {
            LIM_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ", "
                                    << family << ", " << type << ") err=" << error << " errstr="
                                    << gai_strerror(error);
            return false;
        }

        for (p = res; p != nullptr; p = p->ai_next)
        {
            result.push_back(Create(p->ai_addr, (socklen_t)p->ai_addrlen));
        }

        freeaddrinfo(res);
        return !result.empty();
    }

    Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol)
    {
        std::vector<Address::ptr> result;
        if (Lookup(result, host, family, type, protocol))
        {
            return result[0];
        }
        return nullptr;
    }

    std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string &host, int family, int type, int protocol)
    {
        std::vector<Address::ptr> result;
        if (Lookup(result, host, family, type, protocol))
        {
            for (auto &i : result)
            {
                IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
                if (v)
                {
                    return v;
                }
            }
        }
        return nullptr;
    }

    bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family)
    {
        struct ifaddrs *next, *results;
        if (getifaddrs(&results) != 0)
        {
            LIM_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddresses getifaddrs err=" << errno << " errstr=" << strerror(errno);
            return false;
        }

        try
        {
            for (next = results; next; next = next->ifa_next)
            {
                Address::ptr addr;
                uint32_t prefix_len = ~0u;
                if (family != AF_UNSPEC && family != next->ifa_addr->sa_family)
                {
                    continue;
                }
                switch (next->ifa_addr->sa_family)
                {
                case AF_INET:
                {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                    uint32_t netmask = ((sockaddr_in *)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_len = CountBytes(netmask);
                }
                break;
                case AF_INET6:
                {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                    in6_addr &netmask = ((sockaddr_in6 *)next->ifa_netmask)->sin6_addr;
                    prefix_len = 0;
                    for (int i = 0; i < 16; ++i)
                    {
                        prefix_len += CountBytes(netmask.s6_addr[i]);
                    }
                }
                break;
                default:
                    break;
                }

                if (addr)
                {
                    result.insert(std::make_pair(next->ifa_name,
                                                 std::make_pair(addr, prefix_len)));
                }
            }
        }
        catch (...)
        {
            LIM_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
            freeifaddrs(results);
            return false;
        }
        freeifaddrs(results);
        return !result.empty();
    }

    int Address::getFamily() const
    {
        return getAddr()->sa_family;
    }

    std::string Address::toString() const
    {
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }

    bool Address::operator<(const Address &rhs) const
    {
        socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
        int result = memcmp(getAddr(), rhs.getAddr(), minlen);
        if (result != 0)
        {
            return result < 0;
        }

        return getAddrLen() < rhs.getAddrLen();
    }

    bool Address::operator==(const Address &rhs) const
    {
        return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
    }

    bool Address::operator!=(const Address &rhs) const
    {
        return !(*this == rhs);
    }

    IPAddress::ptr IPAddress::Create(const char *address, uint16_t port)
    {
        // 初始化 hints 结构体，并指定 AI_NUMERICHOST 标志以要求解析为数值形式的地址
        addrinfo hints, *results;
        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_flags = AI_NUMERICHOST; // 解析为数值形式的地址
        hints.ai_family = AF_UNSPEC;     // 指定支持的地址族（IPv4、IPv6 或两者都支持）

        // 调用 getaddrinfo 函数解析地址
        int error = getaddrinfo(address, NULL, &hints, &results);
        if (error)
        {
            LIM_LOG_ERROR(g_logger) << "IPAddress::Create(" << address
                                    << ", " << port << ") error=" << error
                                    << " errno=" << errno << " errstr=" << strerror(errno);
            return nullptr;
        }

        try
        {
            // 调用 Address::Create 函数创建地址对象，并传入解析得到的 ai_addr 和 ai_addrlen
            IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
            if (result)
            {
                // 设置端口号
                result->setPort(port);
            }
            // 释放 getaddrinfo 返回的结果内存
            freeaddrinfo(results);
            return result;
        }
        catch (...)
        {
            // 处理异常情况，释放结果内存并返回 nullptr
            freeaddrinfo(results);
            return nullptr;
        }
    }

    IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port)
    {
        IPv4Address::ptr new_addr(new IPv4Address);
        new_addr->m_addr.sin_port = htons(port);
        int rt = inet_pton(AF_INET, address, &new_addr->m_addr.sin_addr);
        if (rt <= 0)
        {
            // 转换失败，输出错误消息
            LIM_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                    << port << ") rt=" << rt << " errno=" << errno
                                    << " errstr=" << strerror(errno);
            return nullptr;
        }
        return new_addr;
    }

    IPv4Address::IPv4Address(uint32_t address, uint32_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = htonl(address);
    }

    IPv4Address::IPv4Address(const sockaddr_in &address)
    {
        m_addr = address;
    }

    const sockaddr *IPv4Address::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }

    sockaddr *IPv4Address::getAddr()
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const
    {
        return sizeof(m_addr);
    }

    std::ostream &IPv4Address::insert(std::ostream &os) const
    {
        char ipv4Str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(m_addr.sin_addr), ipv4Str, INET_ADDRSTRLEN);
        os << ipv4Str << ":" << htons(m_addr.sin_port);
        return os;
    }

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len)
    {
        if (prefix_len > 32)
        {
            return nullptr;
        }

        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr |= htonl(CreateMask<uint32_t>(prefix_len));
        return IPv4Address::ptr(new IPv4Address(baddr));
    }

    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
    {
        if (prefix_len > 32)
        {
            return nullptr;
        }

        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr &= htonl(CreateMask<uint32_t>(prefix_len));
        return IPv4Address::ptr(new IPv4Address(baddr));
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
    {
        sockaddr_in subnet;
        memset(&subnet, 0, sizeof(subnet));
        subnet.sin_family = AF_INET;
        subnet.sin_addr.s_addr = ~htonl(CreateMask<uint32_t>(prefix_len));
        return IPv4Address::ptr(new IPv4Address(subnet));
    }

    uint32_t IPv4Address::getPort() const
    {
        return htons(m_addr.sin_port);
    }

    void IPv4Address::setPort(uint32_t port)
    {
        m_addr.sin_port = htons(port);
    }

    IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port)
    {
        IPv6Address::ptr new_addr(new IPv6Address);
        new_addr->m_addr.sin6_port = htons(port);
        int rt = inet_pton(AF_INET6, address, &new_addr->m_addr.sin6_addr);
        if (rt <= 0)
        {
            // 转换失败，输出错误消息
            LIM_LOG_ERROR(g_logger) << "IPv6Address::Create(" << address << ", "
                                    << port << ") rt=" << rt << " errno=" << errno
                                    << " errstr=" << strerror(errno);
            return nullptr;
        }
        return new_addr;
    }

    IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = htons(port);
        memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
    }

    IPv6Address::IPv6Address(const sockaddr_in6 &address)
    {
        m_addr = address;
    }

    IPv6Address::IPv6Address()
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
    }

    const sockaddr *IPv6Address::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }

    sockaddr *IPv6Address::getAddr()
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t IPv6Address::getAddrLen() const
    {
        return sizeof(m_addr);
    }

    std::ostream &IPv6Address::insert(std::ostream &os) const
    {
        char ipv6Str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(m_addr.sin6_addr), ipv6Str, INET6_ADDRSTRLEN);
        os << "[" << ipv6Str << "]:" << htons(m_addr.sin6_port);
        return os;
    }

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
    {
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i)
        {
            baddr.sin6_addr.s6_addr[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }

    IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len)
    {
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i)
        {
            baddr.sin6_addr.s6_addr[i] = 0x00;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }

    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
    {
        sockaddr_in6 subnet;
        memset(&subnet, 0, sizeof(subnet));
        subnet.sin6_family = AF_INET6;
        subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);

        for (uint32_t i = 0; i < prefix_len / 8; ++i)
        {
            subnet.sin6_addr.s6_addr[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(subnet));
    }

    uint32_t IPv6Address::getPort() const
    {
        return htons(m_addr.sin6_port);
    }

    void IPv6Address::setPort(uint32_t port)
    {
        m_addr.sin6_port = htons(port);
    }

    static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;

    UnixAddress::UnixAddress()
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
    }

    UnixAddress::UnixAddress(const std::string &path)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = path.size() + 1;

        if (!path.empty() && path[0] == '\0')
        {
            --m_length;
        }

        if (m_length > sizeof(m_addr.sun_path))
        {
            throw std::logic_error("path too long");
        }
        memcpy(m_addr.sun_path, path.c_str(), m_length);
        m_length += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr *UnixAddress::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }

    sockaddr *UnixAddress::getAddr()
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t UnixAddress::getAddrLen() const
    {
        return m_length;
    }

    void UnixAddress::setAddrLen(uint32_t v)
    {
        m_length = v;
    }

    std::ostream &UnixAddress::insert(std::ostream &os) const
    {
        if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0')
        {
            return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
        }
        return os << m_addr.sun_path;
    }

    std::string UnixAddress::getPath() const
    {
        return std::string();
    }

    UnknownAddress::UnknownAddress(int family)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sa_family = family;
    }

    UnknownAddress::UnknownAddress(const sockaddr &addr)
    {
        m_addr = addr;
    }

    const sockaddr *UnknownAddress::getAddr() const
    {
        return &m_addr;
    }

    sockaddr *UnknownAddress::getAddr()
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t UnknownAddress::getAddrLen() const
    {
        return sizeof(m_addr);
    }

    std::ostream &UnknownAddress::insert(std::ostream &os) const
    {
        os << "[UnknownAddress family=" << m_addr.sa_family << "]";
        return os;
    }
}