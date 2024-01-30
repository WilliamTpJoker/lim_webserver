#include "FdInfo.h"
#include "Hook.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace lim_webserver
{
    FdInfo::FdInfo(int fd) : m_isSocket(false), m_sysNonblock(false), m_userNonblock(false), m_fd(fd)
    {
        // 获取文件描述符的状态信息
        struct stat fd_stat;
        if (-1 != fstat(m_fd, &fd_stat))
        {
            // 获取成功，判断是否是套接字
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }

        // 如果是套接字
        if (m_isSocket)
        {
            // 获取文件描述符的标志
            int flag = fcntl_f(m_fd, F_GETFL, 0);

            // 如果未设置非阻塞标志，则设置为非阻塞
            if (!(flag & O_NONBLOCK))
            {
                fcntl_f(m_fd, F_SETFL, flag | O_NONBLOCK);
            }
            // 标记系统非阻塞为 true
            m_sysNonblock = true;
        }
    }

    FdInfo::~FdInfo() {}

    bool FdInfo::close() { return false; }

    void FdInfo::setTimeout(int type, uint64_t ms)
    {
        if (type = SO_RCVTIMEO)
        {
            m_recvTimeout = ms;
        }
        else if (type = SO_SNDTIMEO)
        {
            m_sendTimeout = ms;
        }
    }

    uint64_t FdInfo::getTimeout(int type)
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

    FdManager::FdManager() { m_fd_vec.resize(64); }

    void FdManager::insert(int fd)
    {
        RWMutexType::WriteLock lock(m_mutex);
        FdInfo::ptr fdInfo = FdInfo::Create(fd);
        if (fd >= (int)m_fd_vec.size())
        {
            m_fd_vec.resize(fd * 2);
        }
        m_fd_vec[fd] = fdInfo;
    }

    FdInfo::ptr FdManager::get(int fd)
    {
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fd_vec.size() <= fd)
        {
            return nullptr;
        }
        return m_fd_vec[fd];
    }

    void FdManager::del(int fd)
    {
        RWMutexType::WriteLock lock(m_mutex);
        if ((int)m_fd_vec.size() <= fd)
        {
            return;
        }
        m_fd_vec[fd].reset();
    }
}