#ifndef __LIM_FD_MANAGER_H__
#define __LIM_FD_MANAGER_H__

#include <memory>

#include "thread.h"
#include "io_manager.h"
#include "singleton.h"

namespace lim_webserver
{
    class FdCtx : public std::enable_shared_from_this<FdCtx>
    {
    public:
        using ptr = std::shared_ptr<FdCtx>;
        static ptr create(int fd)
        {
            return std::make_shared<FdCtx>(fd);
        }

    public:
        FdCtx(int fd);
        ~FdCtx();

        bool init();
        bool isInit() const { return m_isInit; }
        bool isSocket() const { return m_isSocket; }
        bool isClosed() const { return m_isClosed; }
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
        IoManager *m_iom;
    };

    class FdManager
    {
    public:
        using RWMutexType = RWMutex;

    public:
        FdManager();
        FdCtx::ptr get(int fd, bool auto_creat = false);
        void del(int fd);

    private:
        RWMutexType m_mutex;
        std::vector<FdCtx::ptr> m_fd_list;
    };
    using FdMgr = Singleton<FdManager>;

}

#endif