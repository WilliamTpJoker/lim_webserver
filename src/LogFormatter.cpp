#include "LogFormatter.h"
#include "TimeStamp.h"
#include "Processor.h"

#include <assert.h>
#include <unordered_map>
#include <memory>

namespace lim_webserver
{
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getStream();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getLevelString();
        }
    };

    // TODO:未实现
    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << 0;
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {

            stream << event->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Thread::GetThreadId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Thread::GetThreadName();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Fiber::GetFiberId();
        }
    };

    class CoIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        CoIdFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Processor::GetCurrentTask()->getId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format) {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            time_t time_l = event->getTime();   // 获取时间
            std::string buf = TimeMgr::GetInstance()->getTimeString(time_l);
            stream << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FileNameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '\n';
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : m_string(str) {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << m_string;
        }

    private:
        std ::string m_string;
    };

    class PercentSignFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '%';
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '\t';
        }
    };

    /**
     * %p 输出日志等级
     * %f 输出文件名
     * %l 输出行号
     * %d 输出日志时间
     * %t 输出线程号
     * %N 输出线程名
     * %F 输出纤程号
     * %C 输出协程号
     * %m 输出日志消息
     * %n 输出换行
     * %% 输出百分号
     * %T 输出制表符
     * %r 输出自启动到现在的时间
     * %c 输出日志信息所属的类目
     * */
    thread_local static const std::unordered_map<char, std::shared_ptr<LogFormatter::FormatItem>> format_item_map{
#define FN(CH, ITEM_NAME) \
    {                     \
        CH, std::make_shared<ITEM_NAME>()}
        FN('p', LevelFormatItem),
        FN('f', FileNameFormatItem),
        FN('l', LineFormatItem),
        FN('d', DateTimeFormatItem),
        FN('t', ThreadIdFormatItem),
        FN('N', ThreadNameFormatItem),
        FN('F', FiberIdFormatItem),
        FN('C', CoIdFormatItem),
        FN('m', MessageFormatItem),
        FN('n', NewLineFormatItem),
        FN('%', PercentSignFormatItem),
        FN('T', TabFormatItem),
        FN('r', ElapseFormatItem),
        FN('c', NameFormatItem)
#undef FN
    };

    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
        init();
    }

    void LogFormatter::format(LogStream &stream, LogMessage::ptr message)
    {
        for (auto &i : m_items)
        {
            i->format(stream, message);
        }
    }

    void LogFormatter::init()
    {
        enum class PARSE_STATUS
        {
            SCAN_STATUS,   // 扫描普通字符
            CREATE_STATUS, // 扫描到 %，处理占位符
        };
        PARSE_STATUS STATUS = PARSE_STATUS::SCAN_STATUS;
        size_t str_begin = 0, str_end = 0;
        for (size_t i = 0; i < m_pattern.length(); i++)
        {
            switch (STATUS)
            {
            case PARSE_STATUS::SCAN_STATUS: // 普通扫描分支，将扫描到普通字符串创建对应的普通字符处理对象后填入 m_item 中
                // 扫描记录普通字符的开始结束位置
                str_begin = i;
                for (str_end = i; str_end < m_pattern.length(); str_end++)
                {
                    // 扫描到 % 结束普通字符串查找，将 STATUS 赋值为占位符处理状态，等待后续处理后进入占位符处理状态
                    if (m_pattern[str_end] == '%')
                    {
                        STATUS = PARSE_STATUS::CREATE_STATUS;
                        break;
                    }
                }
                i = str_end;
                m_items.push_back(
                    std::make_shared<StringFormatItem>(
                        m_pattern.substr(str_begin, str_end - str_begin)));
                break;

            case PARSE_STATUS::CREATE_STATUS: // 处理占位符
                assert(!format_item_map.empty() && "format_item_map 没有被正确的初始化");
                auto itor = format_item_map.find(m_pattern[i]);
                if (itor == format_item_map.end())
                {
                    m_error = true;
                    m_items.push_back(std::make_shared<StringFormatItem>("<error format>"));
                }
                else
                {
                    m_items.push_back(itor->second);
                }
                STATUS = PARSE_STATUS::SCAN_STATUS;
                break;
            }
        }
    }

} // namespace lim_webserver
