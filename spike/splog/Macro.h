#pragma once

#include <assert.h>

#include "splog/Util.h"
#include "splog/LogManager.h"

#if defined __GNUC__ || defined __llvm__
#   define LIKELY(x) __builtin_expect(!!(x), 1)
#   define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define LIKELY(x) (x)
#   define UNLIKELY(x) (x)
#endif

/**
 * @brief 断言宏封装
 */
#define ASSERT(condition, ...)                                                  \
    do                                                                              \
    {                                                                               \
        if (!(condition))                                                           \
        {                                                                           \
            LOG_ERROR(LOG_ROOT()) << "\nASSERTION: " << #condition << "\t"  \
                                          << #__VA_ARGS__ << "\nbacktrace:"         \
                                          << lim_webserver::BackTraceToString(100); \
            assert(condition);                                                      \
        }                                                                           \
    } while (false)
    