#pragma once

#include <memory>

#include "Thread.h"
#include "Singleton.h"

#include <vector>

namespace lim_webserver
{
    class FdContext : public std::enable_shared_from_this<FdContext>
    {
    public:
        using ptr = std::shared_ptr<FdContext>;
        static ptr Create(int fd)
        {
            return std::make_shared<FdContext>(fd);
        }

    public:
        FdContext(int fd);
        ~FdContext();

        bool init();
        inline bool isInit() const { return m_isInit; }
        inline bool isSocket() const { return m_isSocket; }
        inline bool isClosed() const { return m_isClosed; }
        bool close();

        bool getUserNonblock() const { return m_userNonblock; }
        bool setUserNonblock(bool v) { m_userNonblock = v; }

        bool getSysNonblock() const { return m_sysNonblock; }
        bool setSysNonblock(bool v) { m_sysNonblock = v; }

        void setTimeout(int type, uint64_t v);
        uint64_t getTimeout(int type);

    private:
        bool m_isInit : 1;
        bool m_isSocket : 1;
        bool m_sysNonblock : 1;
        bool m_userNonblock : 1;
        bool m_isClosed : 1;
        int m_fd;

        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;
    };

    class FdManager : public Singleton<FdManager>
    {
    public:
        using RWMutexType = RWMutex;

    public:
        FdManager();
        FdContext::ptr get(int fd, bool auto_creat = false);
        void del(int fd);

    private:
        RWMutexType m_mutex;
        std::vector<FdContext::ptr> m_fd_list;
    };
}
