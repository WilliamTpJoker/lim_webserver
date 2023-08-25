#ifndef __LIM_MACRO_H__
#define __LIM_MACRO_H__

#include <assert.h>

#include "util.h"
#include "log.h"

/**
 * @brief 断言宏封装
 */
#define LIM_ASSERT(x)                                                           \
    if (!(x))                                                                   \
    {                                                                           \
        LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ASSERTION: " #x                       \
                                      << "\nbacktrace:"                         \
                                      << lim_webserver::BackTraceToString(100); \
        assert(x);                                                              \
    }

/**
 * @brief 断言宏封装
 */
#define LIM_ASSERT2(x, w)                                                       \
    if (!(x))                                                                   \
    {                                                                           \
        LIM_LOG_ERROR(LIM_LOG_ROOT()) << "ASSERTION: " #x                       \
                                      << "\n"                                   \
                                      << w                                      \
                                      << "\nbacktrace:"                         \
                                      << lim_webserver::BackTraceToString(100); \
        assert(x);                                                              \
    }

#endif
