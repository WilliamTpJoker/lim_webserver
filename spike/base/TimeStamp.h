#pragma once
#include <string>
#include <inttypes.h>
#include <sys/time.h>
#include <memory>

#include "Mutex.h"
#include "Singleton.h"

#include <iostream>

namespace lim_webserver
{
    class TimeStamp
    {
    public:
        using ptr = std::shared_ptr<TimeStamp>;
        static const int kMicroSecondsPerSecond = 1000 * 1000;

        /**
         * @brief 获得当前时间
         *
         * @return TimeStamp
         */
        static TimeStamp::ptr now()
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            int64_t seconds = tv.tv_sec;
            return std::make_shared<TimeStamp>(seconds * kMicroSecondsPerSecond + tv.tv_usec);
        }

    public:
        TimeStamp() : m_time(0) {}

        explicit TimeStamp(int64_t time) : m_time(time) {}

        /**
         * @brief 返回微秒
         *
         * @return int64_t
         */
        int64_t us() const { return m_time; }

        /**
         * @brief 返回毫秒
         *
         * @return int64_t
         */
        int64_t ms() const { return m_time / 1000; }

        /**
         * @brief 获得字符串
         *
         * @return std::string
         */
        std::string toString() const
        {
            char buf[32] = {0};
            int64_t seconds = m_time / kMicroSecondsPerSecond;
            int64_t microseconds = m_time % kMicroSecondsPerSecond;
            snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
            return buf;
        }

        /**
         * @brief 获得格式化字符串
         *
         * @param showMicroseconds
         * @return std::string
         */
        std::string toFormattedString(bool showMicroseconds) const
        {
            char buf[64] = {0};
            time_t seconds = static_cast<time_t>(m_time / kMicroSecondsPerSecond);
            struct tm tm_time;
            // 使用 localtime_r 获取本地时间
            localtime_r(&seconds, &tm_time);

            // 定义格式化字符串
            const char *format = showMicroseconds ? "%4d-%02d-%02d %02d:%02d:%02d.%06d" : "%4d-%02d-%02d %02d:%02d:%02d";

            // 格式化时间字符串
            int microseconds = showMicroseconds ? static_cast<int>(m_time % kMicroSecondsPerSecond) : 0;
            snprintf(buf, sizeof(buf), format,
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                     microseconds);
            return buf;
        }

    private:
        int64_t m_time;
    };

    class TimeManager : public Singleton<TimeManager>
    {
        using MutexType = Spinlock;

    public:
        TimeManager()
        {
            formatString(time(0));
        }

        const std::string &getTimeString(time_t seconds)
        {
            if (!isNewestSecond(seconds))
            {
                formatString(seconds);
            }
            MutexType::Lock lock(m_mutex);
            return m_lastTimeString;
        }

    private:
        void formatString(time_t seconds)
        {
            MutexType::Lock lock(m_mutex);
            char buf[64]{0};
            struct tm tm_time;
            // 使用 localtime_r 获取本地时间
            localtime_r(&seconds, &tm_time);
            snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
            m_lastTimeString = buf;
            m_lastSeconds = seconds;
        }

        bool isNewestSecond(time_t seconds)
        {
            MutexType::Lock lock(m_mutex);
            return seconds == m_lastSeconds;
        }

        std::string m_lastTimeString; // 最后一次调用时的字符串
        time_t m_lastSeconds;         // 最后一次调用时的时间
        MutexType m_mutex;
    };
} // namespace lim_webserver
