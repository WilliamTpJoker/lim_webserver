#pragma once

#include <memory>

namespace lim_webserver
{
    class Allocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }

        static void Dealloc(void *vp, size_t size)
        {
            return free(vp);
        }
    };
} // namespace lim_webserver
