#include "ByteArray.h"

#include "Endian.h"
#include "splog.h"

#include <iomanip>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    ByteArray::Node::Node(size_t s)
        : ptr(new char[s]), next(nullptr), size(s)
    {
    }

    ByteArray::Node::Node()
        : ptr(nullptr), next(nullptr), size(0)
    {
    }

    ByteArray::Node::~Node()
    {
        if (ptr)
        {
            delete[] ptr;
        }
    }

    ByteArray::ByteArray(size_t base_size)
        : m_baseSize(base_size), m_capacity(base_size), m_root(new Node(base_size))
    {
        m_writePos = pointer(m_root);
        m_readPos = pointer(m_root);
    }

    ByteArray::~ByteArray()
    {
        Node *cur;
        Node *tmp = m_root;
        while (tmp)
        {
            cur = tmp;
            tmp = tmp->next;
            delete cur;
        }
    }

    void ByteArray::writeFint8(int8_t value)
    {
        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint8(uint8_t value)
    {
        write(&value, sizeof(value));
    }
    void ByteArray::writeFint16(int16_t value)
    {
        int16_t v = endian::hostToNetwork(value);
        write(&v, sizeof(value));
    }

    void ByteArray::writeFuint16(uint16_t value)
    {
        uint16_t v = endian::hostToNetwork(value);
        write(&v, sizeof(value));
    }

    void ByteArray::writeFint32(int32_t value)
    {
        int32_t v = endian::hostToNetwork(value);
        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint32(uint32_t value)
    {
        uint32_t v = endian::hostToNetwork(value);
        write(&v, sizeof(value));
    }

    void ByteArray::writeFint64(int64_t value)
    {
        int64_t v = endian::hostToNetwork(value);
        write(&v, sizeof(value));
    }

    void ByteArray::writeFuint64(uint64_t value)
    {
        uint64_t v = endian::hostToNetwork(value);
        write(&v, sizeof(value));
    }

    static uint32_t EncodeZigzag32(const int32_t &v)
    {
        if (v < 0)
        {
            return ((uint32_t)(-v)) * 2 - 1;
        }
        else
        {
            return v * 2;
        }
    }

    static uint64_t EncodeZigzag64(const int64_t &v)
    {
        if (v < 0)
        {
            return ((uint64_t)(-v)) * 2 - 1;
        }
        else
        {
            return v * 2;
        }
    }

    static int32_t DecodeZigzag32(const uint32_t &v)
    {
        return (v >> 1) ^ -(v & 1);
    }

    static int64_t DecodeZigzag64(const uint64_t &v)
    {
        return (v >> 1) ^ -(v & 1);
    }

    void ByteArray::writeInt32(int32_t value)
    {
        writeUint32(EncodeZigzag32(value));
    }

    void ByteArray::writeUint32(uint32_t value)
    {
        uint8_t tmp[5];
        uint8_t i = 0;
        while (value >= 0x80)
        {
            tmp[i++] = (value & 0x7F) | 0x80;
            value >>= 7;
        }
        tmp[i++] = value;
        write(tmp, i);
    }

    void ByteArray::writeInt64(int64_t value)
    {
        writeUint64(EncodeZigzag64(value));
    }

    void ByteArray::writeUint64(uint64_t value)
    {
        uint8_t tmp[10];
        uint8_t i = 0;
        while (value >= 0x80)
        {
            tmp[i++] = (value & 0x7F) | 0x80;
            value >>= 7;
        }
        tmp[i++] = value;
        write(tmp, i);
    }

    void ByteArray::writeFloat(float value)
    {
        uint32_t v;
        memcpy(&v, &value, sizeof(value));
        writeFuint32(v);
    }

    void ByteArray::writeDouble(double value)
    {
        uint64_t v;
        memcpy(&v, &value, sizeof(value));
        writeFuint64(v);
    }

    void ByteArray::writeStringF16(const std::string &value)
    {
        writeFuint16(value.size());
        write(value.c_str(), value.size());
    }

    void ByteArray::writeStringF32(const std::string &value)
    {
        writeFuint32(value.size());
        write(value.c_str(), value.size());
    }

    void ByteArray::writeStringF64(const std::string &value)
    {
        writeFuint64(value.size());
        write(value.c_str(), value.size());
    }

    void ByteArray::writeStringVint(const std::string &value)
    {
        writeUint64(value.size());
        write(value.c_str(), value.size());
    }

    void ByteArray::writeStringWithoutLength(const std::string &value)
    {
        write(value.c_str(), value.size());
    }

    int8_t ByteArray::readFint8()
    {
        int8_t v;
        read(&v, sizeof(v));
        return v;
    }

    uint8_t ByteArray::readFuint8()
    {
        uint8_t v;
        read(&v, sizeof(v));
        return v;
    }

#define XX(type)         \
    type v;              \
    read(&v, sizeof(v)); \
    return endian::networkToHost(v);

    int16_t ByteArray::readFint16()
    {
        XX(int16_t);
    }
    uint16_t ByteArray::readFuint16()
    {
        XX(uint16_t);
    }

    int32_t ByteArray::readFint32()
    {
        XX(int32_t);
    }

    uint32_t ByteArray::readFuint32()
    {
        XX(uint32_t);
    }

    int64_t ByteArray::readFint64()
    {
        XX(int64_t);
    }

    uint64_t ByteArray::readFuint64()
    {
        XX(uint64_t);
    }

#undef XX

    int32_t ByteArray::readInt32()
    {
        return DecodeZigzag32(readUint32());
    }

    uint32_t ByteArray::readUint32()
    {
        uint32_t result = 0;
        for (int i = 0; i < 32; i += 7)
        {
            uint8_t b = readFuint8();
            if (b < 0x80)
            {
                result |= ((uint32_t)b) << i;
                break;
            }
            else
            {
                result |= (((uint32_t)(b & 0x7f)) << i);
            }
        }
        return result;
    }

    int64_t ByteArray::readInt64()
    {
        return DecodeZigzag64(readUint64());
    }

    uint64_t ByteArray::readUint64()
    {
        uint64_t result = 0;
        for (int i = 0; i < 64; i += 7)
        {
            uint8_t b = readFuint8();
            if (b < 0x80)
            {
                result |= ((uint64_t)b) << i;
                break;
            }
            else
            {
                result |= (((uint64_t)(b & 0x7f)) << i);
            }
        }
        return result;
    }

    float ByteArray::readFloat()
    {
        uint32_t v = readFuint32();
        float value;
        memcpy(&value, &v, sizeof(v));
        return value;
    }

    double ByteArray::readDouble()
    {
        uint64_t v = readFuint64();
        double value;
        memcpy(&value, &v, sizeof(v));
        return value;
    }

    std::string ByteArray::readStringF16()
    {
        uint16_t len = readFuint16();
        std::string buff;
        buff.resize(len);
        read(&buff[0], len);
        return buff;
    }

    std::string ByteArray::readStringF32()
    {
        uint32_t len = readFuint32();
        std::string buff;
        buff.resize(len);
        read(&buff[0], len);
        return buff;
    }

    std::string ByteArray::readStringF64()
    {
        uint64_t len = readFuint64();
        std::string buff;
        buff.resize(len);
        read(&buff[0], len);
        return buff;
    }

    std::string ByteArray::readStringVint()
    {
        uint64_t len = readUint64();
        std::string buff;
        buff.resize(len);
        read(&buff[0], len);
        return buff;
    }

    void ByteArray::clear()
    {
        m_writePos.pos = m_readPos.pos = 0;
        m_capacity = m_baseSize;
        Node *tmp = m_root->next;
        Node *cur;
        while (tmp)
        {
            cur = tmp;
            tmp = tmp->next;
            delete cur;
        }
        m_writePos.cur = m_root;
        m_readPos.cur = m_root;
        m_root->next = nullptr;
    }

    void ByteArray::write(const void *buf, size_t size)
    {
        if (size == 0)
        {
            return;
        }

        // 确保内存足够
        addCapacity(size);

        size_t npos = m_writePos.pos % m_baseSize;
        size_t ncap = m_writePos.cur->size - npos;
        size_t bpos = 0;

        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy(m_writePos.cur->ptr + npos, (const char *)buf + bpos, size);
                if (m_writePos.cur->size == (npos + size))
                {
                    m_writePos.cur = m_writePos.cur->next;
                }
                m_writePos.pos += size;
                bpos += size;
                size = 0;
            }
            else
            {
                memcpy(m_writePos.cur->ptr + npos, (const char *)buf + bpos, ncap);
                m_writePos.pos += ncap;
                bpos += ncap;
                size -= ncap;
                m_writePos.cur = m_writePos.cur->next;
                ncap = m_writePos.cur->size;
                npos = 0;
            }
        }
    }

    void ByteArray::read(void *buf, size_t size)
    {
        if (size > getReadSize())
        {
            throw std::out_of_range("not enough len");
        }

        size_t npos = m_readPos.pos % m_baseSize;
        size_t ncap = m_readPos.cur->size - npos;
        size_t bpos = 0;
        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy((char *)buf + bpos, m_readPos.cur->ptr + npos, size);
                if (m_readPos.cur->size == (npos + size))
                {
                    m_readPos.cur = m_readPos.cur->next;
                }
                m_readPos.pos += size;
                bpos += size;
                size = 0;
            }
            else
            {
                memcpy((char *)buf + bpos, m_readPos.cur->ptr + npos, ncap);
                m_readPos.pos += ncap;
                bpos += ncap;
                size -= ncap;
                m_readPos.cur = m_readPos.cur->next;
                ncap = m_readPos.cur->size;
                npos = 0;
            }
        }
    }

    void ByteArray::read(void *buf, size_t size, size_t position) const
    {
        if (size > (m_writePos.pos - position))
        {
            throw std::out_of_range("not enough len");
        }

        size_t npos = position % m_baseSize;
        size_t ncap = m_readPos.cur->size - npos;
        size_t bpos = 0;
        Node *cur = m_readPos.cur;
        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy((char *)buf + bpos, cur->ptr + npos, size);
                if (cur->size == (npos + size))
                {
                    cur = cur->next;
                }
                position += size;
                bpos += size;
                size = 0;
            }
            else
            {
                memcpy((char *)buf + bpos, cur->ptr + npos, ncap);
                position += ncap;
                bpos += ncap;
                size -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
        }
    }

    bool ByteArray::toFile(const std::string &name) const
    {
        std::ofstream ofs;
        ofs.open(name, std::ios::trunc | std::ios::binary);
        if (!ofs)
        {
            LOG_ERROR(g_logger) << "writeToFile name=" << name
                                << " error , errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }

        int64_t read_size = getReadSize();
        int64_t pos = m_readPos.pos;
        Node *cur = m_readPos.cur;

        while (read_size > 0)
        {
            int diff = pos % m_baseSize;
            int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
            ofs.write(cur->ptr + diff, len);
            cur = cur->next;
            pos += len;
            read_size -= len;
        }

        return true;
    }

    bool ByteArray::fromFile(const std::string &name)
    {
        std::ifstream ifs;
        ifs.open(name, std::ios::binary);
        if (!ifs)
        {
            LOG_ERROR(g_logger) << "readFromFile name=" << name
                                << " error, errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }

        std::shared_ptr<char> buff(new char[m_baseSize], [](char *ptr)
                                   { delete[] ptr; });
        while (!ifs.eof())
        {
            ifs.read(buff.get(), m_baseSize);
            write(buff.get(), ifs.gcount());
        }
        return true;
    }

    void ByteArray::addCapacity(size_t size)
    {
        if (size == 0)
        {
            return;
        }
        size_t old_cap = getCapacity();

        // 若容量足够则不扩容
        if (old_cap >= size)
        {
            return;
        }

        // 计算需要添加的内存块数
        size = size - old_cap;
        size_t count = ceil(1.0 * size / m_baseSize);

        // 以当前写指针为起始
        Node *tmp = m_writePos.cur;

        Node *first = nullptr;
        for (size_t i = 0; i < count; ++i)
        {
            tmp->next = new Node(m_baseSize);
            if (first == nullptr)
            {
                first = tmp->next;
            }
            tmp = tmp->next;
            m_capacity += m_baseSize;
        }

        // 若老容量已经为空，则直接将写指针所指向的内存指向下一块
        if (old_cap == 0)
        {
            m_writePos.cur = first;
        }
    }

    std::string ByteArray::toString() const
    {
        std::string str;
        str.resize(getReadSize());
        if (str.empty())
        {
            return str;
        }
        read(&str[0], str.size(), m_readPos.pos);
        return str;
    }

    std::string ByteArray::toHexString() const
    {
        std::string str = toString();
        std::stringstream ss;

        for (size_t i = 0; i < str.size(); ++i)
        {
            if (i > 0 && i % 32 == 0)
            {
                ss << std::endl;
            }
            ss << std::setw(2) << std::setfill('0') << std::hex
               << (int)(uint8_t)str[i] << " ";
        }

        return ss.str();
    }

    uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len) const
    {
        len = len > getReadSize() ? getReadSize() : len;
        if (len == 0)
        {
            return 0;
        }

        uint64_t size = len;

        size_t npos = m_readPos.pos % m_baseSize; // 计算当前节点内的偏移量
        size_t ncap = m_readPos.cur->size - npos; // 计算当前节点内剩余可读数据长度
        struct iovec iov;                         // 用于构建每个缓冲区的 iovec 结构体
        Node *cur = m_readPos.cur;                // 当前节点指针

        while (len > 0)
        {
            if (ncap >= len)
            {
                // 如果当前节点剩余可读数据足够满足请求
                iov.iov_base = cur->ptr + npos; // 缓冲区的起始位置
                iov.iov_len = len;              // 缓冲区的长度
                len = 0;                        // 已经满足请求，结束循环
            }
            else
            {
                // 如果当前节点剩余可读数据不足以满足请求
                iov.iov_base = cur->ptr + npos; // 缓冲区的起始位置
                iov.iov_len = ncap;             // 缓冲区的长度为当前节点剩余可读数据长度
                len -= ncap;                    // 减去已经满足的长度
                cur = cur->next;                // 移动到下一个节点
                ncap = cur->size;               // 更新当前节点的可读数据长度
                npos = 0;                       // 重置偏移量
            }
            buffers.push_back(iov); // 将构建好的缓冲区信息添加到向量中
        }
        return size; // 返回实际获取的数据长度
    }

    uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len, uint64_t position) const
    {
        len = len > getReadSize() ? getReadSize() : len;
        if (len == 0)
        {
            return 0;
        }

        uint64_t size = len;

        size_t npos = position % m_baseSize;
        size_t count = position / m_baseSize;
        Node *cur = m_root;
        while (count > 0)
        {
            cur = cur->next;
            --count;
        }

        size_t ncap = cur->size - npos;
        struct iovec iov;
        while (len > 0)
        {
            if (ncap >= len)
            {
                iov.iov_base = cur->ptr + npos;
                iov.iov_len = len;
                len = 0;
            }
            else
            {
                iov.iov_base = cur->ptr + npos;
                iov.iov_len = ncap;
                len -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
            buffers.push_back(iov);
        }
        return size;
    }

    uint64_t ByteArray::getWriteBuffers(std::vector<iovec> &buffers, uint64_t len)
    {
        if (len == 0)
        {
            return 0;
        }

        // 确保内存足够
        addCapacity(len);
        uint64_t size = len;

        size_t npos = m_writePos.pos % m_baseSize;
        size_t ncap = m_writePos.cur->size - npos;
        struct iovec iov;
        Node *cur = m_writePos.cur;
        while (len > 0)
        {
            if (ncap >= len)
            {
                iov.iov_base = cur->ptr + npos;
                iov.iov_len = len;
                len = 0;
            }
            else
            {
                iov.iov_base = cur->ptr + npos;
                iov.iov_len = ncap;

                len -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
            buffers.push_back(iov);
        }
        return size;
    }

    void ByteArray::addWritePosition(int len)
    {
        size_t npos = m_writePos.pos % m_baseSize;
        size_t ncap = m_writePos.cur->size - npos;

        while (len > 0)
        {
            if (ncap >= len)
            {
                if (m_writePos.cur->size == (npos + len))
                {
                    m_writePos.cur = m_writePos.cur->next;
                }
                m_writePos.pos += len;
                len = 0;
            }
            else
            {
                m_writePos.pos += ncap;
                len -= ncap;
                m_writePos.cur = m_writePos.cur->next;
                ncap = m_writePos.cur->size;
                npos = 0;
            }
        }
    }
    void ByteArray::addReadPosition(int len)
    {
        size_t npos = m_readPos.pos % m_baseSize;
        size_t ncap = m_readPos.cur->size - npos;

        while (len > 0)
        {
            if (ncap >= len)
            {
                if (m_readPos.cur->size == (npos + len))
                {
                    m_readPos.cur = m_readPos.cur->next;
                }
                m_readPos.pos += len;
                len = 0;
            }
            else
            {
                m_readPos.pos += ncap;
                len -= ncap;
                m_readPos.cur = m_readPos.cur->next;
                ncap = m_readPos.cur->size;
                npos = 0;
            }
        }
    }
} // namespace lim_webserver
