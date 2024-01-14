#include "LogAppender.h"

#include <iostream>
#include <assert.h>

namespace lim_webserver
{
    void LogAppender::doAppend(LogMessage::ptr message)
    {
        if (!isStarted())
        {
            return;
        }
        LogStream logstream;
        format(logstream, message);
        const LogStream::Buffer &buf(logstream.buffer());
        append(buf.data(), buf.length());
    }

    OutputAppender::OutputAppender(const LogAppenderDefine &lad)
    {
        m_name = lad.name;
        m_level = lad.level;
        m_formatter = LogFormatter::Create(lad.formatter);
    }

    void OutputAppender::format(LogStream &logstream, LogMessage::ptr message)
    {
        if (!isStarted())
        {
            return;
        }
        if (message->getLevel() >= m_level)
        {
            m_formatter->format(logstream, message);
        }
    }

    void OutputAppender::append(const char *logline, int len)
    {
        if (!isStarted())
        {
            return;
        }
        m_sink->append(logline, len);
        m_sink->flush();
    }

    void OutputAppender::flush()
    {
        if (!isStarted())
        {
            return;
        }
        m_sink->flush();
    }

    void OutputAppender::start()
    {
        int errors = 0;
        if (m_formatter == nullptr)
        {
            std::cout << "error: No Formatter set for the appender named " << m_name << std::endl;
            ++errors;
        }
        if (m_sink == nullptr)
        {
            std::cout << "error: No Sink set for the appender named " << m_name << std::endl;
            ++errors;
        }
        if (errors == 0)
        {
            LogAppender::start();
        }
    }

    void OutputAppender::stop()
    {
        if (!isStarted())
        {
            return;
        }
        {
            // TODO:关闭流
        }
        LogAppender::stop();
    }

    ConsoleAppender::ConsoleAppender()
    {
        m_sink = ConsoleSink::ptr(new ConsoleSink());
    }

    ConsoleAppender::ConsoleAppender(const LogAppenderDefine &lad)
        : OutputAppender(lad)
    {
        m_sink = ConsoleSink::ptr(new ConsoleSink());
    }

    FileAppender::FileAppender()
    {
        m_sink = FileSink::ptr(new FileSink());
    }

    FileAppender::FileAppender(const std::string &filename, bool append)
        : m_filename(filename), m_append(append)
    {
        m_sink = FileSink::ptr(new FileSink());
    }

    FileAppender::FileAppender(const LogAppenderDefine &lad)
        : OutputAppender(lad)
    {
        m_append = lad.append;
        m_filename = lad.file;
        m_sink = FileSink::ptr(new FileSink());
    }

    void FileAppender::start()
    {
        int errors = 0;
        if (!m_filename.empty())
        {
        }
        openFile();
        if (errors == 0)
        {
            OutputAppender::start();
        }
    }

    RollingFileAppender::RollingFileAppender(const std::string &filename, RollingPolicy::ptr rollingPolicy, TriggeringPolicy::ptr triggeringPolicy)
        : FileAppender(filename, true), m_rollingPolicy(rollingPolicy), m_triggeringPolicy(triggeringPolicy)
    {
    }

    void RollingFileAppender::setRollingPolicy(RollingPolicy::ptr rollingPolicy)
    {
        m_rollingPolicy = rollingPolicy;
        // 如果该策略同时继承了触发策略，则同时也设置触发策略
        TriggeringPolicy::ptr rt = std::dynamic_pointer_cast<TriggeringPolicy>(rollingPolicy);
        if (rt != nullptr)
        {
            m_triggeringPolicy = rt;
        }
    }

    void RollingFileAppender::setTriggeringPolicy(TriggeringPolicy::ptr triggeringPolicy)
    {
        m_triggeringPolicy = triggeringPolicy;
        // 如果该策略同时继承了滚动策略，则同时也设置滚动策略
        RollingPolicy::ptr rt = std::dynamic_pointer_cast<RollingPolicy>(triggeringPolicy);
        if (rt != nullptr)
        {
            m_rollingPolicy = rt;
        }
    }

    void RollingFileAppender::start()
    {
    }

    void RollingFileAppender::format(LogStream &logstream, LogMessage::ptr message)
    {
        if (!isStarted())
        {
            return;
        }
        {
            MutexType::Lock lock(m_trigger_mutex);
            // 判断是否触发了滚动
            if (m_triggeringPolicy->isTriggeringMessage(std::dynamic_pointer_cast<FileSink>(m_sink), message))
            {
                // 滚动
                rollover();
            }
        }
        FileAppender::format(logstream, message);
    }

    void RollingFileAppender::rollover()
    {
        m_rollingPolicy->rollover();
    }

    AsyncAppender::AsyncAppender()
        : m_cond(m_append_mutex)
    {
    }

    AsyncAppender::AsyncAppender(OutputAppender::ptr appender)
        : m_appender(appender), m_cond(m_append_mutex)
    {
    }

    void AsyncAppender::setInterval(int interval)
    {
        m_flushInterval = interval;
    }

    void AsyncAppender::format(LogStream &logstream, LogMessage::ptr message)
    {
        if (!isStarted())
        {
            return;
        }
        m_appender->format(logstream, message);
    }

    void AsyncAppender::append(const char *logline, int len)
    {
        if (!isStarted())
        {
            return;
        }
        Mutex::Lock lock(m_append_mutex);
        // 若当前缓冲区大小支持写入内容则写入
        if (m_buffer.buffer1->avail() > len)
        {
            m_buffer.buffer1->append(logline, len);
        }
        else // 若缓存区不支持写入，则寻找新的缓冲区
        {
            submitBuffer();
            // 在缓冲区内写入内容并提醒后端线程开始写入
            m_buffer.buffer1->append(logline, len);
            m_cond.notify_one();
        }
    }

    void AsyncAppender::submitBuffer()
    {
        m_buffer.buffer_vec.push_back(m_buffer.buffer1);
        // 智能指针的reset，重新指向了新的对象
        m_buffer.buffer1.reset();
        // 若备用缓存区存在，则直接使用
        if (m_buffer.buffer2)
            m_buffer.buffer1 = std::move(m_buffer.buffer2);
        else // 若不存在，则创建新的缓冲区
            m_buffer.buffer1.reset(new Buffer);
    }

    void AsyncAppender::start()
    {
        int errors = 0;
        if (m_appender == nullptr)
        {
            std::cout << "AsyncAppender error: no appender binded" << std::endl;
            ++errors;
        }
        if (errors == 0)
        {
            LogAppender::start();
            m_thread = Thread::Create([this]()
                                      { this->run(); },
                                      m_name);
        }
    }

    void AsyncAppender::stop()
    {
        if (!isStarted())
        {
            return;
        }
        LogAppender::stop();
        {
            Mutex::Lock lock(m_append_mutex);
            submitBuffer();
            m_cond.notify_one();
        }
        m_thread->join();
    }

    void AsyncAppender::bindAppender(OutputAppender::ptr appender)
    {
        m_appender = appender;
    }

    void AsyncAppender::run()
    {
        // 创建新缓存
        DoubleBuffer newBuffer;

        // 当停止时，如果前端缓存仍然有数据则将数据写入后再结束线程
        while (m_started | !m_buffer.empty())
        {
            assert(newBuffer.buffer1 && newBuffer.buffer1->length() == 0);
            assert(newBuffer.buffer2 && newBuffer.buffer2->length() == 0);
            assert(newBuffer.buffer_vec.empty());
            // 进入临界区
            {
                Mutex::Lock lock(m_append_mutex);
                // 若存储区内没有缓存，表明当前缓存没写满，则暂时解锁临界区并等待超时
                // 此处条件变量的唤醒有两种情况：1.超时 2.前端写满并notify
                if (m_buffer.buffer_vec.empty())
                {
                    m_cond.waitTime(m_flushInterval);
                }
                // 此时已经满足上述两条件之一，将缓存存入容器并重置智能指针
                m_buffer.buffer_vec.push_back(m_buffer.buffer1);
                m_buffer.buffer1.reset();
                // 将空缓存分配给当前缓存
                m_buffer.buffer1 = std::move(newBuffer.buffer1);

                // 将新的容器与旧容器指针交换，此时buffersToWrite中存满了数据，而m_buffer_vec则为新容器
                swap(newBuffer.buffer_vec, m_buffer.buffer_vec);

                // 确保前端始终有备用缓存
                if (!m_buffer.buffer2)
                {
                    m_buffer.buffer2 = std::move(newBuffer.buffer2);
                }
            }
            // 至此，含有日志信息的缓存已经不在临界区内，后续的落地操作则不存在线程安全问题

            assert(!newBuffer.buffer_vec.empty());

            // 若内容过多，则可能存在异常，将多余部分删除
            // TODO: 设计方法提示异常
            if (newBuffer.buffer_vec.size() > 25)
                newBuffer.buffer_vec.erase(newBuffer.buffer_vec.begin() + 2, newBuffer.buffer_vec.end());

            // 落地
            for (size_t i = 0; i < newBuffer.buffer_vec.size(); ++i)
                m_appender->append(newBuffer.buffer_vec[i]->data(), newBuffer.buffer_vec[i]->length());

            // 重置容器与缓存
            newBuffer.reset();

            // 刷新日志文件
            m_appender->flush();
        }
        // 后端退出前最后一次刷新
        m_appender->flush();
    }

    FileAppender::ptr AppenderFactory::defaultFileAppender()
    {
        FileAppender::ptr appender = newFileAppender();
        appender->setName("default_file");
        appender->setFormatter(DEFAULT_PATTERN);
        return appender;
    }

    ConsoleAppender::ptr AppenderFactory::defaultConsoleAppender()
    {
        ConsoleAppender::ptr appender = newConsoleAppender();
        appender->setName("default_console");
        appender->setFormatter(DEFAULT_PATTERN);
        return appender;
    }

} // namespace lim_webserver
