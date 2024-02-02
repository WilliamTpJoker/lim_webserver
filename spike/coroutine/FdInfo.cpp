#include "FdInfo.h"
#include "Hook.h"
#include "splog.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    FdInfo::FdInfo(int fd) : IoChannel(fd), m_isSocket(false), m_sysNonblock(false), m_userNonblock(false)
    {
        // 获取文件描述符的状态信息
        struct stat fd_stat;
        if (-1 != fstat(fd, &fd_stat))
        {
            // 获取成功，判断是否是套接字
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }

        // 如果是套接字
        if (m_isSocket)
        {
            // 使用原生函数获取文件描述符的标志
            int flag = fcntl_f(fd, F_GETFL, 0);

            // 如果未设置非阻塞标志，则设置为非阻塞
            if (!(flag & O_NONBLOCK))
            {
                fcntl_f(fd, F_SETFL, flag | O_NONBLOCK);
            }
            // 标记系统非阻塞为 true
            m_sysNonblock = true;
        }
    }

    FdInfo::~FdInfo() {}

    void FdInfo::close()
    {
        IoChannel::close();
        LOG_DEBUG(g_logger) << "Close FdInfo(fd = " << m_fd << ")";
    }

    void FdInfo::setSocketTimeout(int type, uint64_t ms)
    {
        switch (type)
        {
        case SO_RCVTIMEO:
            m_recvTimeout = ms;
            break;
        case SO_SNDTIMEO:
            m_sendTimeout = ms;
            break;
        }
    }

    uint64_t FdInfo::getSocketTimeout(int type)
    {
        switch (type)
        {
        case SO_RCVTIMEO:
            return m_recvTimeout;
        case SO_SNDTIMEO:
            return m_sendTimeout;
        default:
            return 0;
        }
    }

    FdManager::FdManager() {}

    void FdManager::create(int fd)
    {
        FdInfo::ptr fdInfo = FdInfo::Create(fd);
        insert(fd, fdInfo);
    }

    FdInfo::ptr FdManager::get(int fd)
    {
        FdSlotPtr slot = getSlot(fd);
        if (!slot)
            return FdInfo::ptr();

        Spinlock::Lock lock(slot->mutex_);
        FdInfo::ptr info(slot->data_);
        return info;
    }

    void FdManager::del(int fd)
    {
        FdSlotPtr slot = getSlot(fd);
        if (!slot)
            return;

        FdInfo::ptr info;
        {
            Spinlock::Lock lock(slot->mutex_);
            slot->data_.swap(info);
        }

        if (info)
        {
            info->close();
        }
    }

    FdManager::FdSlotPtr FdManager::getSlot(int fd)
    {
        int bucketIdx = fd & kBucketCount;
        std::unique_lock<std::mutex> lock(m_bucketMtx[bucketIdx]);
        auto &bucket = m_buckets[bucketIdx];
        auto itr = bucket.find(fd);
        if (itr == bucket.end())
            return FdSlotPtr();
        return itr->second;
    }

    void FdManager::insert(int fd, FdInfo::ptr info)
    {
        int bucketIdx = fd & kBucketCount;

        std::unique_lock<std::mutex> lock(m_bucketMtx[bucketIdx]);
        auto &bucket = m_buckets[bucketIdx];
        FdSlotPtr &slot = bucket[fd];
        if (!slot)
            slot.reset(new FdSlot);
        lock.unlock();

        FdInfo::ptr closedInfo;
        {
            Spinlock::Lock lock2(slot->mutex_);
            closedInfo.swap(slot->data_);
            slot->data_ = info;
        }

        if (closedInfo)
        {
            closedInfo->close();
        }
    }
}