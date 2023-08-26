#ifndef __LIM_MACRO_H__
#define __LIM_MACRO_H__

#include <assert.h>

#include "util.h"
#include "log.h"

/**
 * @brief 断言宏封装
 */
#define LIM_ASSERT(condition, ...)                                                  \
    do                                                                              \
    {                                                                               \
        if (!(condition))                                                           \
        {                                                                           \
            LIM_LOG_ERROR(LIM_LOG_ROOT()) << "\nASSERTION: " << #condition << "\t"  \
                                          << #__VA_ARGS__ << "\nbacktrace:"         \
                                          << lim_webserver::BackTraceToString(100); \
            assert(condition);                                                      \
        }                                                                           \
    } while (false)

#endif
