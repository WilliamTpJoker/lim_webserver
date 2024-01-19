#include "FdManager.h"
#include "Hook.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace lim_webserver
{
    FdContext::FdContext(int fd)
        : m_isInit(false), m_isSocket(false), m_isClosed(false), m_sysNonblock(false), m_userNonblock(false), m_fd(fd), m_recvTimeout(-1), m_sendTimeout(-1)
    {
        init();
    }

    FdContext::~FdContext()
    {
    }

    bool FdContext::init()
    {
        if (m_isInit)
        {
            return true;
        }
        m_recvTimeout = -1;
        m_sendTimeout = -1;

        struct stat fd_stat;
        if (-1 == fstat(m_fd, &fd_stat))
        {
            m_isInit = false;
            m_isSocket = false;
        }
        else
        {
            m_isInit = true;
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }

        if (m_isSocket)
        {
            int flag = fcntl_f(m_fd, F_GETFL, 0);
            if (!(flag & O_NONBLOCK))
            {
                fcntl_f(m_fd, F_SETFL, flag | O_NONBLOCK);
            }
            m_sysNonblock = true;
        }
        else
        {
            m_sysNonblock = false;
        }

        m_userNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }

    bool FdContext::close()
    {
        return false;
    }

    void FdContext::setTimeout(int type, uint64_t v)
    {
        if (type = SO_RCVTIMEO)
        {
            m_recvTimeout = v;
        }
        else if (type = SO_SNDTIMEO)
        {
            m_sendTimeout = v;
        }
    }

    uint64_t FdContext::getTimeout(int type)
    {
        if (type = SO_RCVTIMEO)
        {
            return m_recvTimeout;
        }
        else if (type = SO_SNDTIMEO)
        {
            return m_sendTimeout;
        }
    }

    FdManager::FdManager()
    {
        m_fd_list.resize(64);
    }

    FdContext::ptr FdManager::get(int fd, bool auto_creat)
    {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (m_fd_list.size() <= fd)
            {
                if (!auto_creat)
                {
                    return nullptr;
                }
            }
            else
            {
                if (m_fd_list[fd] || !auto_creat)
                {
                    return m_fd_list[fd];
                }
            }
        }
        RWMutexType::WriteLock lock2(m_mutex);
        FdContext::ptr ctx = FdContext::Create(fd);
        if (fd >= (int)m_fd_list.size())
        {
            m_fd_list.resize(fd * 1.5);
        }
        m_fd_list[fd] = ctx;
        return ctx;
    }

    void FdManager::del(int fd)
    {
        RWMutexType::WriteLock lock(m_mutex);
        if ((int)m_fd_list.size() <= fd)
        {
            return;
        }
        m_fd_list[fd].reset();
    }
}