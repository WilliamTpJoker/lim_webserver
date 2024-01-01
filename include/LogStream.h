#pragma once

#include <string.h>
#include <string>
#include <stdio.h>
#include <memory>

#include "Noncopyable.h"

namespace lim_webserver
{
    const int kSmallBuffer = 4000;        // 4k
    const int kLargeBuffer = 4000 * 1000; // 4M

    template <int SIZE>
    class FixedBuffer : Noncopyable
    {
    public:
        FixedBuffer()
            : m_cur(m_data)
        {
        }

        ~FixedBuffer()
        {
        }

        // 向缓冲区追加数据
        void append(const char *buf, size_t len)
        {
            if (avail() >= static_cast<int>(len))
            {
                memcpy(m_cur, buf, len);
                m_cur += len;
            }
        }

        // 获取指向缓冲区数据的指针
        const char *data() const { return m_data; }
        // 获取缓冲区中数据的长度
        int length() const { return static_cast<int>(m_cur - m_data); }

        // 获取指向缓冲区当前位置的指针
        char *current() { return m_cur; }

        // 获取缓冲区中的可用空间
        int avail() const { return static_cast<int>(end() - m_cur); }

        // 将当前位置移动指定的长度
        void add(size_t len) { m_cur += len; }

        // 将当前位置重置为缓冲区开头
        void reset() { m_cur = m_data; }

        // 将数据全部置零
        void bzero() { memset(m_data, 0, sizeof(m_data)); }

        // 将缓冲区转换为字符串
        std::string toString() const { return std::string(m_data, length()); }

    private:
        // 获取指向缓冲区末尾的指针
        const char *end() const { return m_data + sizeof(m_data); }

        char m_data[SIZE]; // 用于存储数据的固定大小字符数组
        char *m_cur;       // 指向缓冲区当前位置的指针
    };

    class LogStream : Noncopyable
    {
    
    public:
        using Buffer = FixedBuffer<kSmallBuffer>;

        LogStream &operator<<(bool v)
        {
            m_buffer.append(v ? "1" : "0", 1);
            return *this;
        }

        LogStream &operator<<(short);
        LogStream &operator<<(unsigned short);
        LogStream &operator<<(int);
        LogStream &operator<<(unsigned int);
        LogStream &operator<<(long);
        LogStream &operator<<(unsigned long);
        LogStream &operator<<(long long);
        LogStream &operator<<(unsigned long long);

        LogStream &operator<<(float v)
        {
            *this << static_cast<double>(v);
            return *this;
        }
        LogStream &operator<<(double);
        LogStream &operator<<(long double);

        LogStream &operator<<(char v)
        {
            m_buffer.append(&v, 1);
            return *this;
        }

        LogStream &operator<<(const char *str)
        {
            if (str)
            {
                m_buffer.append(str, strlen(str));
            }
            else
            {
                m_buffer.append("(null)", 6);
            }
            return *this;
        }

        LogStream &operator<<(const unsigned char *str)
        {
            return operator<<(reinterpret_cast<const char *>(str));
        }

        LogStream &operator<<(const std::string &v)
        {
            m_buffer.append(v.c_str(), v.size());
            return *this;
        }

        LogStream &operator<<(LogStream& stream)
        {
            m_buffer.append(stream.buffer().data(),stream.buffer().length());
            return *this;
        }

        void append(const char *data, int len) { m_buffer.append(data, len); }
        const Buffer &buffer() const { return m_buffer; }
        void resetBuffer() { m_buffer.reset(); }

    private:
        template <typename T>
        void formatInteger(T);

        Buffer m_buffer;

        static const int kMaxNumericSize = 48;
    };
} // namespace lim_webserver
