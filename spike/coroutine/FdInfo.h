#pragma once

#include <memory>

#include "base/Mutex.h"
#include "base/Singleton.h"

#include <unordered_map>

namespace lim_webserver
{
    class FdInfo : public std::enable_shared_from_this<FdInfo>
    {
    public:
        using ptr = std::shared_ptr<FdInfo>;
        static ptr Create(int fd)
        {
            return std::make_shared<FdInfo>(fd);
        }

    public:
        FdInfo(int fd);
        ~FdInfo();

        inline bool isSocket() const { return m_isSocket; }
        bool close();

        bool getUserNonblock() const { return m_userNonblock; }
        bool setUserNonblock(bool v) { m_userNonblock = v; }

        bool getSysNonblock() const { return m_sysNonblock; }
        bool setSysNonblock(bool v) { m_sysNonblock = v; }

        /**
         * @brief 设置超时时间(毫秒单位)
         *
         * @param type
         * @param v
         */
        void setTimeout(int type, uint64_t ms);

        /**
         * @brief 获取超时时间
         *
         * @param type 接收或发送
         * @return uint64_t
         */
        uint64_t getTimeout(int type);

        uint64_t getConnectTimeout() { return m_tcpConnectTimeout; }

        void setConnectTimeout(uint64_t ms){m_tcpConnectTimeout = ms;}

    private:
        bool m_isSocket : 1;
        bool m_sysNonblock : 1;
        bool m_userNonblock : 1;
        int m_fd;                          // 句柄
        uint64_t m_recvTimeout = -1;       // 接受超时
        uint64_t m_sendTimeout = -1;       // 发送超时
        uint64_t m_tcpConnectTimeout = -1; // TCP连接超时
    };

    class FdManager : public Singleton<FdManager>
    {
    public:
        using RWMutexType = RWMutex;

    public:
        FdManager();

        /**
         * @brief 通过fd插入FdInfo
         *
         * @param fd
         */
        void insert(int fd);

        FdInfo::ptr get(int fd);
        void del(int fd);

    private:
        RWMutexType m_mutex;
        std::unordered_map<int, FdInfo::ptr> m_fd_map;
    };
}
