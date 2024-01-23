#pragma once

#include <string.h>

namespace lim_webserver
{
    class FileName
    {
    public:
        template <int N>
        FileName(const char (&arr)[N])
            : data_(arr),
              size_(N - 1)
        {
            const char *slash = strrchr(data_, '/'); // builtin function
            if (slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit FileName(const char *filename)
            : data_(filename)
        {
            const char *slash = strrchr(filename, '/');
            if (slash)
            {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char *data_;
        int size_;
    };
} // namespace lim_webserver
