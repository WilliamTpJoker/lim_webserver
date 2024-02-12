#include <execinfo.h>
#include <cxxabi.h>

#include "BackTrace.h"
#include "splog.h"

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_NAME("system");

    static std::string demangle(const char *str)
    {
        size_t size = 0;
        int status = 0;
        std::string rt;
        rt.resize(256);
        if (1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0]))
        {
            char *v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
            std::cout << "1 " << rt <<std::endl;
            if (v)
            {
                std::string result(v);
                free(v);
                return result;
            }
        }
        if (1 == sscanf(str, "%255s", &rt[0]))
        {
            return rt;
        }
        return str;
    }

    void BackTrace(std::vector<std::string> &bt, int maxFrames, int skip)
    {
        // 使用 malloc 分配一段内存来存储调用栈信息
        void **array = (void **)malloc((sizeof(void *) * maxFrames));
        // 获取调用栈信息，存储在 array 中，size 表示获取的栈帧数目
        size_t numFrames = ::backtrace(array, maxFrames);

        // 使用 backtrace_symbols 将地址转换为对应的函数名、源文件和行号
        char **strings = backtrace_symbols(array, numFrames);
        if (strings == NULL)
        {
            LOG_ERROR(g_logger) << "backtrace_symbols error";
            return;
        }

        // 从 skip 处开始将转换后的调用栈信息存储到传入的 bt 向量中
        for (size_t i = skip; i < numFrames; ++i)
        {
            bt.emplace_back(demangle(strings[i]));
        }

        // 释放 backtrace_symbols 分配的内存
        free(strings);
        // 释放之前分配的内存
        free(array);
    }

    std::string BackTraceToString(int size, int skip)
    {
        // 收集函数调用栈
        std::vector<std::string> bt;
        BackTrace(bt, size, skip);

        std::stringstream ss;
        ss << std::endl;

        // 将调用栈转换为格式化的字符串
        for (size_t i = 0; i < bt.size(); ++i)
        {
            ss << bt[i] << std::endl;
        }
        return ss.str();
    }
}