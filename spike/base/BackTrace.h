#pragma once

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <memory>
#include <vector>
#include <sys/time.h>

namespace lim_webserver
{
    /**
     * @brief 收集函数调用栈信息。
     *
     * @param bt    用于存储转换后的调用栈信息的向量。
     * @param size  从调用栈中收集的地址数量。
     * @param skip  跳过的起始栈帧数目，用于排除不必要的信息。
     */
    void BackTrace(std::vector<std::string> &bt, int size, int skip=0);

    /**
     * @brief 将函数调用栈转换为格式化的字符串表示。
     *
     * @param size   从调用栈中收集的地址数量。
     * @param skip   跳过的起始栈帧数目，用于排除不必要的信息。
     * @param prefix 每行前缀，通常是输出的日志级别和位置信息。
     * @return       格式化的字符串，表示调用栈。
     */
    std::string BackTraceToString(int size=64, int skip=0);
}