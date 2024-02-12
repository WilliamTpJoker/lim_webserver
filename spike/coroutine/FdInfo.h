#pragma once

#include "base/Mutex.h"
#include "base/Singleton.h"
#include "net/IoChannel.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace lim_webserver
{
    class FdInfo : public IoChannel
    {
    public:
        using ptr = std::shared_ptr<FdInfo>;
        static ptr Create(int fd) { return std::make_shared<FdInfo>(fd); }

    public:
        FdInfo(int fd);
        ~FdInfo();

        inline bool isSocket() const { return m_isSocket; }
        void close() override;

        bool getUserNonblock() const { return m_userNonblock; }
        void setUserNonblock(bool v) { m_userNonblock = v; }

        bool getSysNonblock() const { return m_sysNonblock; }
        void setSysNonblock(bool v) { m_sysNonblock = v; }

        /**
         * @brief 设置超时时间(毫秒单位)
         *
         * @param type
         * @param ms
         */
        void setSocketTimeout(int type, uint64_t ms);

        /**
         * @brief 获取超时时间(毫秒单位)
         *
         * @param type 接收或发送
         * @return uint64_t
         */
        uint64_t getSocketTimeout(int type);

        /**
         * @brief Get the Tcp Connect Timeout(ms)
         *
         * @return uint64_t
         */
        inline uint64_t getTcpConnectTimeout() const { return m_tcpConnectTimeout; }

        /**
         * @brief Set the Tcp Connect Timeout(ms)
         *
         * @param ms
         */
        inline void setTcpConnectTimeout(uint64_t ms) { m_tcpConnectTimeout = ms; }

    private:
        bool m_isSocket : 1;
        bool m_sysNonblock : 1;
        bool m_userNonblock : 1;
        uint64_t m_recvTimeout = -1;       // 接受超时
        uint64_t m_sendTimeout = -1;       // 发送超时
        uint64_t m_tcpConnectTimeout = -1; // TCP连接超时
    };

    class FdManager : public Singleton<FdManager>
    {
    public:
        using RWMutexType = RWMutex;
        static const int kStaticFdSize = 128;
        static const int kBucketShift = 10;
        static const int kBucketCount = (1 << kBucketShift) - 1;

        struct FdSlot
        {
            FdInfo::ptr data_;
            Spinlock mutex_;
        };
        using FdSlotPtr = std::shared_ptr<FdSlot>;

    public:
        FdManager();

        /**
         * @brief 通过fd插入FdInfo
         *
         * @param fd
         */
        void create(int fd);

        FdInfo::ptr get(int fd);
        void del(int fd);

    private:
        FdSlotPtr getSlot(int fd);

        void insert(int fd, FdInfo::ptr info);

    private:
        using Slots = std::unordered_map<int, FdSlotPtr>;
        std::mutex m_bucketMtx[kBucketCount + 1];
        Slots m_buckets[kBucketCount + 1];
    };
}
