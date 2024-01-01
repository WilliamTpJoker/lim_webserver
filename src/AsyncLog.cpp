#include "AsyncLog.h"
#include <assert.h>

namespace lim_webserver
{
    

    void AsyncLog::append(const char *logline, int len)
    {
        MutexType::Lock lock(m_mutex);
        // 若当前缓冲区大小支持写入内容则写入
        if (m_current_buffer->avail() > len)
            m_current_buffer->append(logline, len);
        else // 若缓存区不支持写入，则寻找新的缓冲区
        {
            m_buffer_vec.push_back(m_current_buffer);
            // 智能指针的reset，重新指向了新的对象
            m_current_buffer.reset();
            // 若备用缓存区存在，则直接使用
            if (m_next_buffer)
                m_current_buffer = std::move(m_next_buffer);
            else // 若不存在，则创建新的缓冲区
                m_current_buffer.reset(new Buffer);
            // 在缓冲区内写入内容并提醒后端线程开始写入
            m_current_buffer->append(logline, len);
            m_cond.notify_one();
        }
    }

    void AsyncLog::run()
    {
        LogFile output(m_basename);
        // 创建两个新缓存用以替换原缓存
        BufferPtr newBuffer1(new Buffer);
        BufferPtr newBuffer2(new Buffer);
        newBuffer1->bzero();
        newBuffer2->bzero();

        // 创建存储缓存的容器
        BufferVec buffersToWrite;
        buffersToWrite.reserve(16);

        while (m_running)
        {
            assert(newBuffer1 && newBuffer1->length() == 0);
            assert(newBuffer2 && newBuffer2->length() == 0);
            assert(buffersToWrite.empty());
            // 进入临界区
            {
                MutexType::Lock lock(m_mutex);
                // 若存储区内没有缓存，表明当前缓存没写满，则暂时解锁临界区并等待超时
                // 此处条件变量的唤醒有两种情况：1.超时 2.前端写满并notify
                if (m_buffer_vec.empty())
                    m_cond.waitForSeconds(m_flushInterval);
                // 此时已经满足上述两条件之一，将缓存存入容器并重置智能指针
                m_buffer_vec.push_back(m_current_buffer);
                m_current_buffer.reset();
                // 将空缓存分配给当前缓存
                m_current_buffer = std::move(newBuffer1);

                // 将新的容器与旧容器指针交换，此时buffersToWrite中存满了数据，而m_buffer_vec则为新容器
                swap(buffersToWrite, m_buffer_vec);

                // 确保前端始终有备用缓存
                if (!m_next_buffer)
                    m_next_buffer = std::move(newBuffer2);
            }
            // 至此，含有日志信息的缓存已经不在临界区内，后续的落地操作则不存在线程安全问题

            assert(!buffersToWrite.empty());

            // 若内容过多，则可能存在异常，将多余部分删除
            // TODO: 设计方法提示异常
            if (buffersToWrite.size() > 25)
                buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());

            // 落地
            for (size_t i = 0; i < buffersToWrite.size(); ++i)
                output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());

            // 重置容器与缓存
            if (buffersToWrite.size() > 2)
                buffersToWrite.resize(2);
            if (!newBuffer1)
            {
                newBuffer1 = buffersToWrite.back();
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }
            if (!newBuffer2)
            {
                newBuffer2 = buffersToWrite.back();
                buffersToWrite.pop_back();
                newBuffer2->reset();
            }
            buffersToWrite.clear();

            // 刷新日志文件
            output.flush();
        }
        // 后端退出前最后一次刷新
        output.flush();
    }

} // namespace lim_webserver
